#include "ScenarioPanel.h"
#include "ScenarioDialog.h"
#include "../style/AppStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

ScenarioPanel::ScenarioPanel(AppController* controller, QWidget* parent)
    : QWidget(parent)
    , m_ctrl(controller)
{
    setupUi();
    refreshList();

    connect(m_ctrl, &AppController::scenarioFinished,
        this, &ScenarioPanel::onScenarioFinished);
    connect(m_ctrl, &AppController::scenarioStopped,
        this, &ScenarioPanel::onScenarioStopped);
}

void ScenarioPanel::setupUi()
{
    QGroupBox* group = new QGroupBox("Сценарии", this);

    // ── Список сценариев ─────────────────────────────────────────────────────
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);

    // ── Описание и шаги выбранного сценария ──────────────────────────────────
    m_descEdit = new QTextEdit(this);
    m_descEdit->setReadOnly(true);
    m_descEdit->setMaximumHeight(50);
    m_descEdit->setPlaceholderText("Описание сценария...");

    m_stepsWidget = new QListWidget(this);
    m_stepsWidget->setMaximumHeight(120);

    // ── Кнопки управления ────────────────────────────────────────────────────
    m_runBtn = new QPushButton("Выполнить", this);
    m_stopBtn = new QPushButton("Стоп", this);
    m_addBtn = new QPushButton("Добавить", this);
    m_editBtn = new QPushButton("Изменить", this);
    m_deleteBtn = new QPushButton("Удалить", this);

    AppStyle::applyAccent(m_runBtn);
    AppStyle::applyDanger(m_stopBtn);
    AppStyle::applyDanger(m_deleteBtn);

    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addWidget(m_runBtn);
    btnRow->addWidget(m_stopBtn);
    btnRow->addWidget(m_addBtn);
    btnRow->addWidget(m_editBtn);
    btnRow->addWidget(m_deleteBtn);

    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(new QLabel("Доступные сценарии:", this));
    groupLayout->addWidget(m_listWidget, 1);
    groupLayout->addWidget(new QLabel("Описание сценария:", this));
    groupLayout->addWidget(m_descEdit);
    groupLayout->addWidget(new QLabel("Команды в сценарии:", this));
    groupLayout->addWidget(m_stepsWidget);
    groupLayout->addLayout(btnRow);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(group);
    main->setContentsMargins(0, 0, 0, 0);

    connect(m_listWidget, &QListWidget::itemSelectionChanged,
        this, &ScenarioPanel::onSelectionChanged);
    connect(m_addBtn, &QPushButton::clicked, this, &ScenarioPanel::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &ScenarioPanel::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &ScenarioPanel::onDeleteClicked);
    connect(m_runBtn, &QPushButton::clicked, this, &ScenarioPanel::onRunClicked);
    connect(m_stopBtn, &QPushButton::clicked, this, &ScenarioPanel::onStopClicked);

    setConnected(false);
    onSelectionChanged();
}

void ScenarioPanel::setConnected(bool connected)
{
    m_connected = connected;
    const bool hasSel = selectedId() >= 0;
    m_runBtn->setEnabled(connected && hasSel && !m_ctrl->isScenarioRunning());
    m_stopBtn->setEnabled(m_ctrl->isScenarioRunning());
}

void ScenarioPanel::refreshList()
{
    const int prevId = selectedId();
    m_listWidget->clear();

    for (const Scenario& s : m_ctrl->scenarios()) {
        QListWidgetItem* item = new QListWidgetItem(s.name, m_listWidget);
        item->setData(Qt::UserRole, s.id);
    }

    // Восстановить выделение
    for (int i = 0; i < m_listWidget->count(); ++i) {
        if (m_listWidget->item(i)->data(Qt::UserRole).toInt() == prevId) {
            m_listWidget->setCurrentRow(i);
            break;
        }
    }
}

int ScenarioPanel::selectedId() const
{
    QListWidgetItem* item = m_listWidget->currentItem();
    return item ? item->data(Qt::UserRole).toInt() : -1;
}

void ScenarioPanel::setRunning(bool running)
{
    m_runBtn->setEnabled(!running && m_connected && selectedId() >= 0);
    m_stopBtn->setEnabled(running);
    m_addBtn->setEnabled(!running);
    m_editBtn->setEnabled(!running);
    m_deleteBtn->setEnabled(!running);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void ScenarioPanel::onSelectionChanged()
{
    const int id = selectedId();
    const bool hasSel = id >= 0;

    m_editBtn->setEnabled(hasSel);
    m_deleteBtn->setEnabled(hasSel);
    m_runBtn->setEnabled(hasSel && m_connected && !m_ctrl->isScenarioRunning());

    // Обновляем описание и шаги
    m_descEdit->clear();
    m_stepsWidget->clear();

    if (!hasSel) return;

    auto s = std::find_if(m_ctrl->scenarios().begin(), m_ctrl->scenarios().end(),
        [id](const Scenario& sc) { return sc.id == id; });
    if (s == m_ctrl->scenarios().end()) return;

    m_descEdit->setPlainText(s->description);
    for (int i = 0; i < s->steps.size(); ++i)
        m_stepsWidget->addItem(QString("%1. %2").arg(i + 1).arg(s->steps[i]));
}

void ScenarioPanel::onAddClicked()
{
    ScenarioDialog dlg(m_ctrl->commands(), this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_ctrl->addScenario(dlg.result());
    refreshList();
    emit statusMessage("Сценарий добавлен");
}

void ScenarioPanel::onEditClicked()
{
    const int id = selectedId();
    if (id < 0) return;

    auto it = std::find_if(m_ctrl->scenarios().begin(), m_ctrl->scenarios().end(),
        [id](const Scenario& s) { return s.id == id; });
    if (it == m_ctrl->scenarios().end()) return;

    ScenarioDialog dlg(*it, m_ctrl->commands(), this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_ctrl->updateScenario(dlg.result());
    refreshList();
    onSelectionChanged();
    emit statusMessage("Сценарий обновлён");
}

void ScenarioPanel::onDeleteClicked()
{
    const int id = selectedId();
    if (id < 0) return;

    const auto reply = QMessageBox::question(
        this, "Удалить сценарий",
        QString("Удалить \"%1\"?").arg(m_listWidget->currentItem()->text()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    m_ctrl->removeScenario(id);
    refreshList();
    emit statusMessage("Сценарий удалён");
}

void ScenarioPanel::onRunClicked()
{
    const int id = selectedId();
    if (id < 0 || !m_connected) return;

    m_ctrl->runScenario(id);
    setRunning(true);
    emit statusMessage(QString("Сценарий запущен: %1")
        .arg(m_listWidget->currentItem()->text()));
}

void ScenarioPanel::onStopClicked()
{
    m_ctrl->stopScenario();
}

void ScenarioPanel::onScenarioFinished(const QString& name)
{
    setRunning(false);
    emit statusMessage(QString("Сценарий завершён: %1").arg(name));
}

void ScenarioPanel::onScenarioStopped(const QString& name)
{
    setRunning(false);
    emit statusMessage(QString("Сценарий остановлен: %1").arg(name));
}