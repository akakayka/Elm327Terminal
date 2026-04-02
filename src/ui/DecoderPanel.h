#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>

#include "../controllers/AppController.h"

// DecoderPanel — таблица фильтров декодирования CAN сообщений.
// Каждая строка: CAN ID | SubID | Название
class DecoderPanel : public QWidget {
    Q_OBJECT

public:
    explicit DecoderPanel(AppController* controller, QWidget* parent = nullptr);

signals:
    void statusMessage(const QString& message);

private slots:
    void onAddClicked();
    void onDeleteClicked();
    void onEnabledToggled(bool enabled);
    void onCellChanged(int row, int col);

private:
    void setupUi();
    void refreshTable();

    AppController* m_ctrl;
    bool           m_updating = false;

    QCheckBox* m_enabledCheck;
    QTableWidget* m_table;
    QPushButton* m_addBtn;
    QPushButton* m_deleteBtn;
};