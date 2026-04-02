#include "DecoderPanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>

DecoderPanel::DecoderPanel(AppController* controller, QWidget* parent)
    : QWidget(parent), m_ctrl(controller)
{
    setupUi();
    refreshTable();
}

void DecoderPanel::setupUi()
{
    QGroupBox* group = new QGroupBox("Декодер CAN", this);

    // ── Чекбокс включения ────────────────────────────────────────────────────
    m_enabledCheck = new QCheckBox("Декодировать входящие данные", this);
    m_enabledCheck->setChecked(m_ctrl->isDecoderEnabled());

    // ── Таблица фильтров ─────────────────────────────────────────────────────
    m_table = new QTableWidget(0, 3, this);
    m_table->setHorizontalHeaderLabels({ "CAN ID", "Sub ID", "Название параметра" });
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);

    // ── Кнопки ───────────────────────────────────────────────────────────────
    m_addBtn = new QPushButton("Добавить", this);
    m_deleteBtn = new QPushButton("Удалить", this);

    QLabel* hint = new QLabel(
        "CAN ID: например 18FF97  |  Sub ID: hex, например 01  |  "
        "Для 18FF93 Sub ID оставьте пустым", this);
    hint->setWordWrap(true);
    hint->setStyleSheet("color: gray; font-size: 10px;");

    QHBoxLayout* btnRow = new QHBoxLayout;
    btnRow->addWidget(m_addBtn);
    btnRow->addWidget(m_deleteBtn);
    btnRow->addStretch();

    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->addWidget(m_enabledCheck);
    groupLayout->addWidget(m_table, 1);
    groupLayout->addLayout(btnRow);
    groupLayout->addWidget(hint);

    QVBoxLayout* main = new QVBoxLayout(this);
    main->addWidget(group);
    main->setContentsMargins(0, 0, 0, 0);

    connect(m_enabledCheck, &QCheckBox::toggled,
        this, &DecoderPanel::onEnabledToggled);
    connect(m_addBtn, &QPushButton::clicked, this, &DecoderPanel::onAddClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &DecoderPanel::onDeleteClicked);
    connect(m_table, &QTableWidget::cellChanged,
        this, &DecoderPanel::onCellChanged);
}

void DecoderPanel::refreshTable()
{
    m_updating = true;
    m_table->setRowCount(0);

    const auto& params = m_ctrl->decoderParameters();
    for (auto it = params.begin(); it != params.end(); ++it) {
        const QString& canId = it.key();
        const auto& subMap = it.value();

        for (auto sit = subMap.begin(); sit != subMap.end(); ++sit) {
            const int     subId = sit.key();
            const QString name = sit.value().name;

            const int row = m_table->rowCount();
            m_table->insertRow(row);
            m_table->setItem(row, 0, new QTableWidgetItem(canId));
            m_table->setItem(row, 1, new QTableWidgetItem(
                subId >= 0 ? QString("%1").arg(subId, 2, 16, QChar('0')).toUpper() : ""));
            m_table->setItem(row, 2, new QTableWidgetItem(name));
        }
    }

    m_updating = false;
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void DecoderPanel::onEnabledToggled(bool enabled)
{
    m_ctrl->setDecoderEnabled(enabled);
    emit statusMessage(enabled ? "Декодер включён" : "Декодер выключен");
}

void DecoderPanel::onAddClicked()
{
    m_updating = true;
    const int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem("18FF97"));
    m_table->setItem(row, 1, new QTableWidgetItem("01"));
    m_table->setItem(row, 2, new QTableWidgetItem("Новый параметр"));
    m_updating = false;

    // Применяем сразу
    onCellChanged(row, 0);
    m_table->setCurrentCell(row, 2);
    m_table->editItem(m_table->item(row, 2));
}

void DecoderPanel::onDeleteClicked()
{
    const int row = m_table->currentRow();
    if (row < 0) return;

    const QString canId = m_table->item(row, 0) ? m_table->item(row, 0)->text().trimmed() : "";
    const QString subHex = m_table->item(row, 1) ? m_table->item(row, 1)->text().trimmed() : "";
    const int     subId = subHex.isEmpty() ? -1 : subHex.toInt(nullptr, 16);

    m_ctrl->removeDecoderParameter(canId, subId);
    m_table->removeRow(row);
    emit statusMessage("Параметр удалён");
}

void DecoderPanel::onCellChanged(int row, int /*col*/)
{
    if (m_updating) return;

    const QString canId = m_table->item(row, 0) ? m_table->item(row, 0)->text().trimmed() : "";
    const QString subHex = m_table->item(row, 1) ? m_table->item(row, 1)->text().trimmed() : "";
    const QString name = m_table->item(row, 2) ? m_table->item(row, 2)->text().trimmed() : "";

    if (canId.isEmpty() || name.isEmpty()) return;

    const int subId = subHex.isEmpty() ? -1 : subHex.toInt(nullptr, 16);

    CanParameter param;
    param.name = name;

    // Передаём старые значения для удаления если ID изменился
    m_ctrl->setDecoderParameter(canId, subId, param, canId, subId);
    emit statusMessage(QString("Параметр обновлён: %1").arg(name));
}