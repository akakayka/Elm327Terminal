#include "ScenarioDialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>

ScenarioDialog::ScenarioDialog(const QList<Command>& commands, QWidget* parent)
    : QDialog(parent), m_commands(commands)
{
    setupUi();
}

ScenarioDialog::ScenarioDialog(const Scenario& scenario,
    const QList<Command>& commands,
    QWidget* parent)
    : QDialog(parent), m_commands(commands), m_original(scenario)
{
    setupUi();
    m_nameEdit->setText(scenario.name);
    m_descEdit->setPlainText(scenario.description);
    m_delaySpin->setValue(scenario.delayMs);
    populateSteps(scenario.steps);
}

void ScenarioDialog::setupUi()
{
    setWindowTitle("Редактор сценария");
    setMinimumSize(700, 500);
    setModal(true);

    // ── Верхняя часть: имя, описание, задержка ───────────────────────────────
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Название сценария");

    m_descEdit = new QTextEdit(this);
    m_descEdit->setPlaceholderText("Описание (необязательно)");
    m_descEdit->setMaximumHeight(60);

    m_delaySpin = new QSpinBox(this);
    m_delaySpin->setRange(100, 30000);
    m_delaySpin->setValue(1000);
    m_delaySpin->setSuffix(" мс");
    m_delaySpin->setSingleStep(100);

    QFormLayout* topForm = new QFormLayout;
    topForm->addRow("Название:", m_nameEdit);
    topForm->addRow("Описание:", m_descEdit);
    topForm->addRow("Задержка между командами:", m_delaySpin);

    // ── Нижняя часть: два списка ─────────────────────────────────────────────
    // Левый — доступные команды
    QGroupBox* leftGroup = new QGroupBox("Доступные команды", this);
    m_commandList = new QListWidget(this);
    for (const Command& cmd : m_commands)
        m_commandList->addItem(QString("%1 - %2").arg(cmd.text).arg(cmd.name));

    QPushButton* addBtn = new QPushButton("Добавить →", this);
    addBtn->setToolTip("Добавить выбранную команду в сценарий");

    QVBoxLayout* leftLayout = new QVBoxLayout(leftGroup);
    leftLayout->addWidget(m_commandList, 1);
    leftLayout->addWidget(addBtn);

    // Правый — шаги сценария
    QGroupBox* rightGroup = new QGroupBox("Шаги сценария", this);
    m_stepList = new QListWidget(this);

    QPushButton* removeBtn = new QPushButton("Удалить", this);
    QPushButton* upBtn = new QPushButton("▲ Вверх", this);
    QPushButton* downBtn = new QPushButton("▼ Вниз", this);

    QHBoxLayout* stepBtns = new QHBoxLayout;
    stepBtns->addWidget(upBtn);
    stepBtns->addWidget(downBtn);
    stepBtns->addWidget(removeBtn);

    QVBoxLayout* rightLayout = new QVBoxLayout(rightGroup);
    rightLayout->addWidget(m_stepList, 1);
    rightLayout->addLayout(stepBtns);

    QHBoxLayout* listsRow = new QHBoxLayout;
    listsRow->addWidget(leftGroup, 1);
    listsRow->addWidget(rightGroup, 1);

    // ── Кнопки OK/Cancel ─────────────────────────────────────────────────────
    m_buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    // ── Главный layout ───────────────────────────────────────────────────────
    QVBoxLayout* main = new QVBoxLayout(this);
    main->addLayout(topForm);
    main->addLayout(listsRow, 1);
    main->addWidget(m_buttons);

    // ── Сигналы ──────────────────────────────────────────────────────────────
    connect(addBtn, &QPushButton::clicked, this, &ScenarioDialog::onAddStep);
    connect(removeBtn, &QPushButton::clicked, this, &ScenarioDialog::onRemoveStep);
    connect(upBtn, &QPushButton::clicked, this, &ScenarioDialog::onMoveUp);
    connect(downBtn, &QPushButton::clicked, this, &ScenarioDialog::onMoveDown);

    // Двойной клик по команде = добавить шаг
    connect(m_commandList, &QListWidget::itemDoubleClicked,
        this, &ScenarioDialog::onAddStep);

    connect(m_nameEdit, &QLineEdit::textChanged, this, &ScenarioDialog::validate);
    connect(m_stepList->model(), &QAbstractItemModel::rowsInserted,
        this, &ScenarioDialog::validate);
    connect(m_stepList->model(), &QAbstractItemModel::rowsRemoved,
        this, &ScenarioDialog::validate);

    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    validate();
}

void ScenarioDialog::populateSteps(const QStringList& steps)
{
    m_stepList->clear();
    for (int i = 0; i < steps.size(); ++i) {
        const QString label = QString("%1. %2").arg(i + 1).arg(steps[i]);
        QListWidgetItem* item = new QListWidgetItem(label, m_stepList);
        item->setData(Qt::UserRole, steps[i]); // храним чистый текст команды
    }
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void ScenarioDialog::onAddStep()
{
    QListWidgetItem* item = m_commandList->currentItem();
    if (!item) return;

    // Берём команду из выбранного: "ATZ - Сброс адаптера" → "ATZ"
    const int cmdIndex = m_commandList->row(item);
    if (cmdIndex < 0 || cmdIndex >= m_commands.size()) return;
    const QString cmdText = m_commands[cmdIndex].text;

    // Добавляем в список шагов
    const int stepNum = m_stepList->count() + 1;
    QListWidgetItem* stepItem = new QListWidgetItem(
        QString("%1. %2").arg(stepNum).arg(cmdText), m_stepList);
    stepItem->setData(Qt::UserRole, cmdText);
}

void ScenarioDialog::onRemoveStep()
{
    const int row = m_stepList->currentRow();
    if (row < 0) return;
    delete m_stepList->takeItem(row);

    // Перенумеруем
    for (int i = 0; i < m_stepList->count(); ++i)
        m_stepList->item(i)->setText(
            QString("%1. %2").arg(i + 1)
            .arg(m_stepList->item(i)->data(Qt::UserRole).toString()));
}

void ScenarioDialog::onMoveUp()
{
    const int row = m_stepList->currentRow();
    if (row <= 0) return;
    QListWidgetItem* item = m_stepList->takeItem(row);
    m_stepList->insertItem(row - 1, item);
    m_stepList->setCurrentRow(row - 1);

    // Перенумеруем
    for (int i = 0; i < m_stepList->count(); ++i)
        m_stepList->item(i)->setText(
            QString("%1. %2").arg(i + 1)
            .arg(m_stepList->item(i)->data(Qt::UserRole).toString()));
}

void ScenarioDialog::onMoveDown()
{
    const int row = m_stepList->currentRow();
    if (row < 0 || row >= m_stepList->count() - 1) return;
    QListWidgetItem* item = m_stepList->takeItem(row);
    m_stepList->insertItem(row + 1, item);
    m_stepList->setCurrentRow(row + 1);

    // Перенумеруем
    for (int i = 0; i < m_stepList->count(); ++i)
        m_stepList->item(i)->setText(
            QString("%1. %2").arg(i + 1)
            .arg(m_stepList->item(i)->data(Qt::UserRole).toString()));
}

void ScenarioDialog::validate()
{
    const bool ok = !m_nameEdit->text().trimmed().isEmpty()
        && m_stepList->count() > 0;
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(ok);
}

// ── Result ────────────────────────────────────────────────────────────────────

Scenario ScenarioDialog::result() const
{
    Scenario s;
    s.id = m_original.id;
    s.name = m_nameEdit->text().trimmed();
    s.description = m_descEdit->toPlainText().trimmed();
    s.delayMs = m_delaySpin->value();

    for (int i = 0; i < m_stepList->count(); ++i)
        s.steps << m_stepList->item(i)->data(Qt::UserRole).toString();

    return s;
}