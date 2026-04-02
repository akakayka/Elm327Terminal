#include "TerminalPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>
#include <QDateTime>
#include <QScrollBar>
#include <QFont>
#include <QDebug>
#include <QTimer>

TerminalPanel::TerminalPanel(QWidget* parent)
    : QWidget(parent)
{
    QGroupBox* group = new QGroupBox("Терминал", this);

    m_output = new QPlainTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(10000);

    QFont font("Courier New", 10);
    font.setFixedPitch(true);
    m_output->setFont(font);

    m_clearBtn = new QPushButton("Очистить", this);
    m_saveBtn = new QPushButton("Сохранить лог", this);
    m_autoscrollCheck = new QCheckBox("Автопрокрутка", this);
    m_autoscrollCheck->setChecked(true);

    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addWidget(m_clearBtn);
    btnRow->addWidget(m_saveBtn);
    btnRow->addStretch();
    btnRow->addWidget(m_autoscrollCheck);

    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(m_output, 1);
    groupLayout->addLayout(btnRow);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(group);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    connect(m_clearBtn, &QPushButton::clicked, this, &TerminalPanel::onClearClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &TerminalPanel::onSaveClicked);
}

// ── Public slots ──────────────────────────────────────────────────────────────

void TerminalPanel::onCommandSent(const QString& text)
{
    appendSent(text);
}

void TerminalPanel::onDataReceived(const QString& data)
{
    // Добавляем сырые данные в буфер и выводим в консоль для отладки
    qDebug().noquote() << "[RAW]" << QString(data).replace('\r', "\\r").replace('\n', "\\n");

    m_receiveBuffer += data;

    while (true) {
        int sepPos = -1;
        QChar sepChar;
        for (int i = 0; i < m_receiveBuffer.size(); ++i) {
            QChar c = m_receiveBuffer[i];
            if (c == '\r' || c == '\n' || c == '>') {
                sepPos = i;
                sepChar = c;
                break;
            }
        }

        if (sepPos < 0)
            break;

        const QString token = m_receiveBuffer.left(sepPos).trimmed();

        int nextPos = sepPos + 1;
        if (sepChar != '>' && nextPos < m_receiveBuffer.size()) {
            const QChar next = m_receiveBuffer[nextPos];
            if ((next == '\r' || next == '\n') && next != sepChar)
                nextPos++;
        }
        m_receiveBuffer = m_receiveBuffer.mid(nextPos);

        // '>' означает что адаптер готов к следующей команде.
        // singleShot(0) откладывает сигнал до следующей итерации event loop —
        // это гарантирует что весь текущий chunk данных обработается и выведется
        // прежде чем уйдёт следующая команда сценария.
        if (sepChar == '>') {
            QTimer::singleShot(0, this, &TerminalPanel::adapterReady);
            if (!token.isEmpty())
                appendReceived(token);
            continue;
        }

        if (!token.isEmpty())
            appendReceived(token);
    }
}

void TerminalPanel::onConnectionError(const QString& message)
{
    appendSystem(QString("ОШИБКА: %1").arg(message));
}

void TerminalPanel::onSystemMessage(const QString& message)
{
    appendSystem(message);
}

void TerminalPanel::onScenarioStep(const QString& cmd, int stepNum, int total,
    const QString& scenarioName)
{
    m_output->appendHtml(
        QString("<span style='color:#888888;font-style:italic;'>"
            "[%1] Шаг %2/%3 &nbsp;</span>"
            "<span style='color:#00aa00;'>&gt;&gt;&gt; %4</span>")
        .arg(scenarioName.toHtmlEscaped())
        .arg(stepNum)
        .arg(total)
        .arg(cmd.toHtmlEscaped()));

    scrollToBottom();
}

void TerminalPanel::onScenarioFinished(const QString& scenarioName)
{
    appendSystem(QString("=== Сценарий завершён: %1 ===").arg(scenarioName));
}

void TerminalPanel::onScenarioStopped(const QString& scenarioName)
{
    appendSystem(QString("=== Сценарий остановлен: %1 ===").arg(scenarioName));
}

void TerminalPanel::onDecodedValue(const QString& formatted)
{
    // Декодированные значения — оранжевый цвет, отступ чтобы визуально
    // было понятно что это расшифровка предыдущей строки
    m_output->appendHtml(
        QString("<span style='color:#cc7700;'>%1</span>")
        .arg(formatted.toHtmlEscaped()));
    scrollToBottom();
}

// ── Private slots ─────────────────────────────────────────────────────────────

void TerminalPanel::onClearClicked()
{
    m_output->clear();
    m_receiveBuffer.clear();
}

void TerminalPanel::onSaveClicked()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "Сохранить лог",
        QString("elm327_log_%1.txt")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        "Text files (*.txt)");

    if (path.isEmpty()) return;

    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_output->toPlainText();
    }
}

// ── Вывод ─────────────────────────────────────────────────────────────────────

void TerminalPanel::appendSent(const QString& text)
{
    m_output->appendHtml(
        QString("<span style='color:#00aa00;'>>>> %1</span>")
        .arg(text.toHtmlEscaped()));
    scrollToBottom();
}

void TerminalPanel::appendReceived(const QString& text)
{
    m_output->appendHtml(
        QString("<span style='color:#0088cc;'>&lt;&lt;&lt; %1</span>")
        .arg(text.toHtmlEscaped()));
    scrollToBottom();
}

void TerminalPanel::appendSystem(const QString& text)
{
    m_output->appendHtml(
        QString("<span style='color:#cc0000;'>[%1]</span>")
        .arg(text.toHtmlEscaped()));
    scrollToBottom();
}

void TerminalPanel::scrollToBottom()
{
    if (m_autoscrollCheck->isChecked())
        m_output->verticalScrollBar()->setValue(
            m_output->verticalScrollBar()->maximum());
}