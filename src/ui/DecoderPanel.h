#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include "../controllers/AppController.h"

class DecoderPanel : public QWidget {
    Q_OBJECT

public:
    explicit DecoderPanel(AppController* controller, QWidget* parent = nullptr);

signals:
    void statusMessage(const QString& message);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onToggleEnabled();
    void onDecoderToggled(bool checked);
    void onSelectionChanged();

private:
    void setupUi();
    void refreshTable();

    AppController* m_ctrl;

    QCheckBox* m_enabledCheck;
    QTableWidget* m_table;
    QPushButton* m_addBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_toggleBtn;  // Вкл/Выкл выбранный фильтр
};