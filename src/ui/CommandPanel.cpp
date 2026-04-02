#include "CommandPanel.h"
#include "CommandDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QListWidgetItem>

CommandPanel::CommandPanel(AppController* controller, QWidget* parent)
    : QWidget(parent)
    , m_ctrl(controller)
{
    setupUi();
    refreshList();
}

// ── UI ────────────────────────────────────────────────────────────────────────

void CommandPanel::setupUi()
{
    QGroupBox* group = new QGroupBox("Команды", this);

    // ── Поиск ────────────────────────────────────────────────────────────────
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Поиск команд...");
    m_searchEdit->setClearButtonEnabled(true);

    // ── Список ───────────────────────────────────────────────────────────────
    m_listWidget = new QListWidget(this);
    m_listWidget->setAlternatingRowColors(true);

    // ── Кнопки управления ────────────────────────────────────────────────────
    m_sendBtn = new QPushButton("Отправить", this);
    m_addBtn = new QPushButton("Добавить", this);
    m_editBtn = new QPushButton("Изменить", this);
    m_deleteBtn = new QPushButton("Удалить", this);

    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addWidget(m_sendBtn);
    btnRow->addWidget(m_addBtn);
    btnRow->addWidget(m_editBtn);
    btnRow->addWidget(m_deleteBtn);

    // ── Быстрая команда ──────────────────────────────────────────────────────
    m_quickEdit = new QLineEdit(this);
    m_quickEdit->setPlaceholderText("Введите команду...");

    m_quickSendBtn = new QPushButton("Отправить", this);

    QHBoxLayout* quickRow = new QHBoxLayout;
    quickRow->addWidget(new QLabel("Быстрая команда:", this));
    quickRow->addWidget(m_quickEdit, 1);
    quickRow->addWidget(m_quickSendBtn);

    // ── Сборка ───────────────────────────────────────────────────────────────
    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(new QLabel("Поиск:", this));
    groupLayout->addWidget(m_searchEdit);
    groupLayout->addWidget(new QLabel("Доступные команды:", this));
    groupLayout->addWidget(m_listWidget, 1);
    groupLayout->addLayout(btnRow);
    groupLayout->addLayout(quickRow);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(group);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ── Начальное состояние ──────────────────────────────────────────────────
    setConnected(false);
    onSelectionChanged(); // кнопки под текущее выделение

    // ── Сигналы ──────────────────────────────────────────────────────────────
    connect(m_addBtn, &QPushButton::clicked, this, &CommandPanel::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &CommandPanel::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &CommandPanel::onDeleteClicked);
    connect(m_sendBtn, &QPushButton::clicked, this, &CommandPanel::onSendClicked);

    connect(m_quickSendBtn, &QPushButton::clicked,
        this, &CommandPanel::onQuickSendClicked);
    connect(m_quickEdit, &QLineEdit::returnPressed,
        this, &CommandPanel::onQuickSendClicked);

    connect(m_searchEdit, &QLineEdit::textChanged,
        this, &CommandPanel::onSearchChanged);
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
        this, &CommandPanel::onSelectionChanged);

    // Двойной клик = отправить
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
        this, [this]() { onSendClicked(); });
}

// ── Публичные методы ──────────────────────────────────────────────────────────

void CommandPanel::setConnected(bool connected)
{
    m_connected = connected;
    m_sendBtn->setEnabled(connected && m_listWidget->currentRow() >= 0);
    m_quickSendBtn->setEnabled(connected);
    m_quickEdit->setEnabled(connected);
}

// ── Список ────────────────────────────────────────────────────────────────────

void CommandPanel::refreshList(const QString& filter)
{
    // Запомним выбранный ID чтобы восстановить после обновления
    const int selectedId = selectedCommandId();

    m_listWidget->clear();

    for (const Command& cmd : m_ctrl->commands()) {
        // Фильтр по имени или тексту команды
        if (!filter.isEmpty()) {
            const bool match = cmd.name.contains(filter, Qt::CaseInsensitive)
                || cmd.text.contains(filter, Qt::CaseInsensitive);
            if (!match) continue;
        }

        // Отображаем: "ATZ - Сброс адаптера"
        const QString label = QString("%1 - %2").arg(cmd.text).arg(cmd.name);
        QListWidgetItem* item = new QListWidgetItem(label, m_listWidget);
        item->setData(Qt::UserRole, cmd.id); // храним ID в item
    }

    // Восстановить выделение
    if (selectedId >= 0) {
        for (int i = 0; i < m_listWidget->count(); ++i) {
            if (m_listWidget->item(i)->data(Qt::UserRole).toInt() == selectedId) {
                m_listWidget->setCurrentRow(i);
                break;
            }
        }
    }
}

int CommandPanel::selectedCommandId() const
{
    QListWidgetItem* item = m_listWidget->currentItem();
    if (!item) return -1;
    return item->data(Qt::UserRole).toInt();
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void CommandPanel::onSelectionChanged()
{
    const bool hasSelection = m_listWidget->currentRow() >= 0;
    m_editBtn->setEnabled(hasSelection);
    m_deleteBtn->setEnabled(hasSelection);
    m_sendBtn->setEnabled(hasSelection && m_connected);
}

void CommandPanel::onSearchChanged(const QString& text)
{
    refreshList(text);
}

void CommandPanel::onAddClicked()
{
    CommandDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    m_ctrl->addCommand(dlg.name(), dlg.text());
    refreshList(m_searchEdit->text());
    emit statusMessage(QString("Команда \"%1\" добавлена").arg(dlg.name()));
}

void CommandPanel::onEditClicked()
{
    const int id = selectedCommandId();
    if (id < 0) return;

    // Найдём команду через store внутри контроллера
    const auto& cmds = m_ctrl->commands();
    auto it = std::find_if(cmds.begin(), cmds.end(),
        [id](const Command& c) { return c.id == id; });
    if (it == cmds.end()) return;

    CommandDialog dlg(*it, this);
    if (dlg.exec() != QDialog::Accepted) return;

    Command updated = *it;
    updated.name = dlg.name();
    updated.text = dlg.text();
    m_ctrl->updateCommand(updated);

    refreshList(m_searchEdit->text());
    emit statusMessage(QString("Команда \"%1\" обновлена").arg(updated.name));
}

void CommandPanel::onDeleteClicked()
{
    const int id = selectedCommandId();
    if (id < 0) return;

    QListWidgetItem* item = m_listWidget->currentItem();
    const auto reply = QMessageBox::question(
        this, "Удалить команду",
        QString("Удалить \"%1\"?").arg(item->text()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    m_ctrl->removeCommand(id);
    refreshList(m_searchEdit->text());
    emit statusMessage("Команда удалена");
}

void CommandPanel::onSendClicked()
{
    const int id = selectedCommandId();
    if (id < 0) return;

    if (!m_connected) {
        emit statusMessage("Нет подключения");
        return;
    }

    m_ctrl->sendById(id);

    // Найдём текст команды для лога
    const auto& cmds = m_ctrl->commands();
    auto it = std::find_if(cmds.begin(), cmds.end(),
        [id](const Command& c) { return c.id == id; });
    const QString cmdText = (it != cmds.end()) ? it->text : "?";

    emit commandSent(cmdText);
    emit statusMessage(QString("Отправлено: %1").arg(cmdText));
}

void CommandPanel::onQuickSendClicked()
{
    const QString text = m_quickEdit->text().trimmed();
    if (text.isEmpty()) return;

    if (!m_connected) {
        emit statusMessage("Нет подключения");
        return;
    }

    m_ctrl->sendRaw(text);
    emit commandSent(text);
    emit statusMessage(QString("Отправлено: %1").arg(text));
}