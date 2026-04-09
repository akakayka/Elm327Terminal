#pragma once
#include "TitleBar.h"

#include <QMainWindow>
#include <QLabel>
#include <QTabWidget>
#include <QAction>

#include "../controllers/AppController.h"
#include "ConnectionPanel.h"
#include "CommandPanel.h"
#include "ScenarioPanel.h"
#include "TerminalPanel.h"
#include "DecoderPanel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(AppController* controller, QWidget* parent = nullptr);



private slots:
    void onConnected(const QString& port, int baudRate);
    void onDisconnected();
    void onStatusMessage(const QString& message);
    void onLoadFile();
    void onSaveFile();

private:
    void setupUi();
    void setupMenu();

    AppController* m_ctrl;
    ConnectionPanel* m_connectionPanel;
    CommandPanel* m_commandPanel;
    ScenarioPanel* m_scenarioPanel;
    DecoderPanel* m_decoderPanel;
    TerminalPanel* m_terminalPanel;
    TitleBar* m_titleBar;

    QLabel* m_statusLabel;
    QLabel* m_portLabel;

    bool m_dragging = false;
    QPoint m_dragPosition;
};