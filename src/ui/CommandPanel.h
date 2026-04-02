#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>

#include "../controllers/AppController.h"

// CommandPanel — панель управления командами.
// Список команд, CRUD через диалог, быстрая отправка.
class CommandPanel : public QWidget {
    Q_OBJECT

public:
    explicit CommandPanel(AppController* controller, QWidget* parent = nullptr);

    // Вызывать когда меняется состояние подключения
    void setConnected(bool connected);
    void refreshList(const QString& filter = "");

signals:
    void commandSent(const QString& cmdText);
    void statusMessage(const QString& message);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSendClicked();
    void onQuickSendClicked();
    void onSearchChanged(const QString& text);
    void onSelectionChanged();

private:
    void setupUi();
    int  selectedCommandId() const; // -1 если ничего не выбрано

    AppController* m_ctrl;
    bool           m_connected = false;

    QLineEdit* m_searchEdit;
    QListWidget* m_listWidget;

    QPushButton* m_sendBtn;
    QPushButton* m_addBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;

    QLineEdit* m_quickEdit;
    QPushButton* m_quickSendBtn;
};