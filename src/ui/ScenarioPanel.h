#pragma once

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>

#include "../controllers/AppController.h"

class ScenarioPanel : public QWidget {
    Q_OBJECT

public:
    explicit ScenarioPanel(AppController* controller, QWidget* parent = nullptr);

    void setConnected(bool connected);
    void refreshList();

signals:
    void statusMessage(const QString& message);

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onRunClicked();
    void onStopClicked();
    void onSelectionChanged();
    void onScenarioFinished(const QString& name);
    void onScenarioStopped(const QString& name);

private:
    void setupUi();
    int  selectedId() const;
    void setRunning(bool running);

    AppController* m_ctrl;
    bool           m_connected = false;

    QListWidget* m_listWidget;
    QTextEdit* m_descEdit;    // описание выбранного сценария
    QListWidget* m_stepsWidget; // шаги выбранного сценария (только чтение)

    QPushButton* m_runBtn;
    QPushButton* m_stopBtn;
    QPushButton* m_addBtn;
    QPushButton* m_editBtn;
    QPushButton* m_deleteBtn;
};