#include "DecoderPanel.h"
#include "../style/AppStyle.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QCheckBox>

DecoderPanel::DecoderPanel(AppController* controller, QWidget* parent)
    : QWidget(parent)
    , m_ctrl(controller)
{
    setupUi();
    refreshTable();
}

void DecoderPanel::setupUi()
{
    QGroupBox* group = new QGroupBox(this);

    // ── Глобальный переключатель декодера ────────────────────────────────────
    m_enabledCheck = new QCheckBox("Включить декодирование", this);
    m_enabledCheck->setChecked(m_ctrl->isDecoderEnabled());

    // ── Подсказка по синтаксису ───────────────────────────────────────────────
    QLabel* hint = new QLabel(
        "<b>Переменные:</b> b0..b7 — байты данных&nbsp;&nbsp;"
        "<b>Функции:</b> ieee754(b0,b1,b2,b3) · ieee754le(b0,b1,b2,b3) · "
        "hex2dec(b0,b1) · hex2dec4(b0,b1,b2,b3) · abs · sqrt · round · min · max · pow",
        this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #8A8278; font-size: 11px; padding: 4px 0;");

    // ── Таблица фильтров ──────────────────────────────────────────────────────
    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({ "CAN ID", "Название", "Ед.", "Формула", "Вкл" });
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    m_table->setColumnWidth(0, 90);
    m_table->setColumnWidth(2, 60);
    m_table->setColumnWidth(4, 45);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);

    // ── Кнопки ───────────────────────────────────────────────────────────────
    m_addBtn = new QPushButton("Добавить", this);
    m_editBtn = new QPushButton("Изменить", this);
    m_deleteBtn = new QPushButton("Удалить", this);
    m_toggleBtn = new QPushButton("Вкл/Выкл", this);

    AppStyle::applyAccent(m_addBtn);
    AppStyle::applyDanger(m_deleteBtn);

    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addWidget(m_addBtn);
    btnRow->addWidget(m_editBtn);
    btnRow->addWidget(m_toggleBtn);
    btnRow->addWidget(m_deleteBtn);
    btnRow->addStretch();

    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(m_enabledCheck);
    groupLayout->addWidget(hint);
    groupLayout->addWidget(m_table, 1);
    groupLayout->addLayout(btnRow);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(group);
    main->setContentsMargins(0, 0, 0, 0);

    connect(m_enabledCheck, &QCheckBox::toggled,
        this, &DecoderPanel::onDecoderToggled);
    connect(m_addBtn, &QPushButton::clicked, this, &DecoderPanel::onAddClicked);
    connect(m_editBtn, &QPushButton::clicked, this, &DecoderPanel::onEditClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &DecoderPanel::onDeleteClicked);
    connect(m_toggleBtn, &QPushButton::clicked, this, &DecoderPanel::onToggleEnabled);
    connect(m_table, &QTableWidget::itemDoubleClicked,
        this, [this]() { onEditClicked(); });
    connect(m_table, &QTableWidget::itemSelectionChanged,
        this, &DecoderPanel::onSelectionChanged);

    onSelectionChanged();
}

void DecoderPanel::refreshTable()
{
    m_table->setRowCount(0);
    const auto& filters = m_ctrl->filterStore()->filters();

    for (const CanFilter& f : filters) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        auto* idItem = new QTableWidgetItem(f.canId);
        auto* nameItem = new QTableWidgetItem(f.name);
        auto* unitItem = new QTableWidgetItem(f.unit);
        auto* frmItem = new QTableWidgetItem(f.formula);
        auto* enItem = new QTableWidgetItem(f.enabled ? "✓" : "—");

        enItem->setTextAlignment(Qt::AlignCenter);
        if (!f.enabled) {
            for (auto* item : { idItem, nameItem, unitItem, frmItem, enItem })
                item->setForeground(QColor("#B0A898"));
        }

        m_table->setItem(row, 0, idItem);
        m_table->setItem(row, 1, nameItem);
        m_table->setItem(row, 2, unitItem);
        m_table->setItem(row, 3, frmItem);
        m_table->setItem(row, 4, enItem);

        // Храним id фильтра
        idItem->setData(Qt::UserRole, f.id);
    }
}

void DecoderPanel::onDecoderToggled(bool checked)
{
    m_ctrl->setDecoderEnabled(checked);
    emit statusMessage(checked ? "Декодер включён" : "Декодер выключен");
}

void DecoderPanel::onSelectionChanged()
{
    const bool has = m_table->currentRow() >= 0;
    m_editBtn->setEnabled(has);
    m_deleteBtn->setEnabled(has);
    m_toggleBtn->setEnabled(has);
}

// ── Диалог добавления/редактирования ─────────────────────────────────────────

static CanFilter showFilterDialog(const CanFilter& initial, QWidget* parent)
{
    QDialog dlg(parent);
    dlg.setWindowTitle(initial.id == 0 ? "Добавить фильтр" : "Изменить фильтр");
    dlg.setMinimumWidth(480);

    QLineEdit* canEdit = new QLineEdit(initial.canId, &dlg);
    QLineEdit* nameEdit = new QLineEdit(initial.name, &dlg);
    QLineEdit* unitEdit = new QLineEdit(initial.unit, &dlg);
    QLineEdit* frmEdit = new QLineEdit(initial.formula, &dlg);

    canEdit->setPlaceholderText("Например: 18FF97");
    nameEdit->setPlaceholderText("Название параметра");
    unitEdit->setPlaceholderText("°C, А, у.е. ...");
    frmEdit->setPlaceholderText("ieee754(b0,b1,b2,b3)");
    frmEdit->setFont(QFont("JetBrains Mono,Consolas,monospace", 11));

    // Подсказка с примерами формул
    QLabel* examples = new QLabel(
        "<b>Примеры:</b><br>"
        "<code>ieee754(b0,b1,b2,b3)</code> — big-endian float<br>"
        "<code>hex2dec(b0,b1)</code> — два байта в число<br>"
        "<code>b0 - 40</code> — байт со смещением<br>"
        "<code>(b0 * 256.0 + b1) * 0.1</code> — два байта с масштабом<br>"
        "<code>b1</code> — просто один байт",
        &dlg);
    examples->setWordWrap(true);
    examples->setStyleSheet("color: #8A8278; font-size: 11px;"
        " background: #EAE6E0; border-radius: 4px;"
        " padding: 8px;");

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    AppStyle::applyAccent(buttons->button(QDialogButtonBox::Ok));

    QFormLayout* form = new QFormLayout;
    form->setSpacing(8);
    form->addRow("CAN ID:", canEdit);
    form->addRow("Название:", nameEdit);
    form->addRow("Единицы:", unitEdit);
    form->addRow("Формула:", frmEdit);

    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    layout->setSpacing(10);
    layout->addLayout(form);
    layout->addWidget(examples);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    // Валидация — OK только если CAN ID и формула заполнены
    auto validate = [&]() {
        buttons->button(QDialogButtonBox::Ok)->setEnabled(
            !canEdit->text().trimmed().isEmpty() &&
            !frmEdit->text().trimmed().isEmpty());
        };
    QObject::connect(canEdit, &QLineEdit::textChanged, validate);
    QObject::connect(frmEdit, &QLineEdit::textChanged, validate);
    validate();

    CanFilter result = initial;
    if (dlg.exec() == QDialog::Accepted) {
        result.canId = canEdit->text().trimmed().toUpper().remove(' ');
        result.name = nameEdit->text().trimmed();
        result.unit = unitEdit->text().trimmed();
        result.formula = frmEdit->text().trimmed();
    }
    return result;
}

void DecoderPanel::onAddClicked()
{
    CanFilter blank;
    blank.id = 0;
    blank.enabled = true;

    CanFilter result = showFilterDialog(blank, this);
    if (!result.isValid()) return;

    m_ctrl->filterStore()->addFilter(result);
    refreshTable();
    emit statusMessage(QString("Фильтр %1 добавлен").arg(result.canId));
}

void DecoderPanel::onEditClicked()
{
    const int row = m_table->currentRow();
    if (row < 0) return;

    const int id = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    auto opt = m_ctrl->filterStore()->findById(id);
    if (!opt.has_value()) return;

    CanFilter result = showFilterDialog(*opt, this);
    if (!result.isValid()) return;

    m_ctrl->filterStore()->updateFilter(result);
    refreshTable();
    emit statusMessage(QString("Фильтр %1 обновлён").arg(result.canId));
}

void DecoderPanel::onDeleteClicked()
{
    const int row = m_table->currentRow();
    if (row < 0) return;

    const QString canId = m_table->item(row, 0)->text();
    const auto reply = QMessageBox::question(
        this, "Удалить фильтр",
        QString("Удалить фильтр для %1?").arg(canId),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    const int id = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    m_ctrl->filterStore()->removeById(id);
    refreshTable();
    emit statusMessage("Фильтр удалён");
}

void DecoderPanel::onToggleEnabled()
{
    const int row = m_table->currentRow();
    if (row < 0) return;

    const int id = m_table->item(row, 0)->data(Qt::UserRole).toInt();
    auto opt = m_ctrl->filterStore()->findById(id);
    if (!opt.has_value()) return;

    CanFilter f = *opt;
    f.enabled = !f.enabled;
    m_ctrl->filterStore()->updateFilter(f);
    refreshTable();
    emit statusMessage(QString("Фильтр %1 %2")
        .arg(f.canId)
        .arg(f.enabled ? "включён" : "выключен"));
}