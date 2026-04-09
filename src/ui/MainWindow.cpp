#include "MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QStatusBar>
#include <QSplitter>
#include <QTabWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include "../style/AppStyle.h"

MainWindow::MainWindow(AppController* controller, QWidget* parent)
    : QMainWindow(parent)
    , m_ctrl(controller)
{
    
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowTitle("ELM327 Terminal");
    setMinimumSize(960, 680);
    resize(1200, 780);

    //setupMenu();
    setupUi();
}

void MainWindow::setupUi()
{
    // Создаем центральный виджет с прозрачным фоном
    QWidget* central = new QWidget(this);
    central->setObjectName("centralWidget");
    central->setStyleSheet(R"(
        #centralWidget {
            background-color: #F0EDE8;
            border-radius: 12px;
            border: 1px solid #D8D2C8;
        }
    )");
    setCentralWidget(central);

    // Главный вертикальный layout для центрального виджета
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // ========== ДОБАВИТЬ TITLEBAR ==========
    m_titleBar = new TitleBar(this);
    mainLayout->addWidget(m_titleBar);
    // ======================================

    // Контентный виджет (все остальное)
    QWidget* contentWidget = new QWidget(this);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(8);
    contentLayout->setContentsMargins(12, 10, 12, 12);

    // Панель подключения
    m_connectionPanel = new ConnectionPanel(m_ctrl, this);
    contentLayout->addWidget(m_connectionPanel);

    // Сплиттер
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);

    QTabWidget* tabs = new QTabWidget(this);
    m_commandPanel = new CommandPanel(m_ctrl, this);
    m_scenarioPanel = new ScenarioPanel(m_ctrl, this);
    m_decoderPanel = new DecoderPanel(m_ctrl, this);
    tabs->addTab(m_commandPanel, "Команды");
    tabs->addTab(m_scenarioPanel, "Сценарии");
    tabs->addTab(m_decoderPanel, "Декодер");

    m_terminalPanel = new TerminalPanel(this);

    splitter->addWidget(tabs);
    splitter->addWidget(m_terminalPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);

    contentLayout->addWidget(splitter, 1);

    mainLayout->addWidget(contentWidget);

    // Статусбар
    QWidget* leftSpacer = new QWidget();
    leftSpacer->setFixedWidth(10);
    QWidget* rightSpacer = new QWidget();
    rightSpacer->setFixedWidth(10);
    rightSpacer->setStyleSheet("background-color: transparent;");
    m_statusLabel = new QLabel("○ Не подключено", this);
    m_statusLabel->setStyleSheet("color: #8A8278;");
    m_portLabel = new QLabel("", this);
    m_portLabel->setStyleSheet(
        "color: #8A8278;"
        "font-family: 'JetBrains Mono', 'Consolas', monospace;"
        "font-size: 11px;"
    );


    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->setSizeGripEnabled(false);
    statusBar()->addPermanentWidget(m_portLabel);
    statusBar()->addPermanentWidget(rightSpacer);

    // ========== ДОБАВИТЬ КОННЕКТЫ ДЛЯ TITLEBAR ==========
    connect(m_titleBar, &TitleBar::maximizeRequested, [this]() {
        m_titleBar->updateMaximizeButton(isMaximized());
        });
    // ===================================================

    // Сигналы
    connect(m_connectionPanel, &ConnectionPanel::connected,
        this, &MainWindow::onConnected);
    connect(m_connectionPanel, &ConnectionPanel::disconnected,
        this, &MainWindow::onDisconnected);
    connect(m_connectionPanel, &ConnectionPanel::statusMessage,
        this, &MainWindow::onStatusMessage);

    connect(m_commandPanel, &CommandPanel::statusMessage,
        this, &MainWindow::onStatusMessage);
    connect(m_scenarioPanel, &ScenarioPanel::statusMessage,
        this, &MainWindow::onStatusMessage);
    connect(m_decoderPanel, &DecoderPanel::statusMessage,
        this, &MainWindow::onStatusMessage);

    connect(m_ctrl, &AppController::dataReceived,
        m_terminalPanel, &TerminalPanel::onDataReceived);
    connect(m_ctrl, &AppController::connectionError,
        m_terminalPanel, &TerminalPanel::onConnectionError);
    connect(m_ctrl, &AppController::commandSent,
        m_terminalPanel, &TerminalPanel::onCommandSent);
    connect(m_ctrl, &AppController::scenarioStepSent,
        m_terminalPanel, &TerminalPanel::onScenarioStep);
    connect(m_ctrl, &AppController::scenarioFinished,
        m_terminalPanel, &TerminalPanel::onScenarioFinished);
    connect(m_ctrl, &AppController::scenarioStopped,
        m_terminalPanel, &TerminalPanel::onScenarioStopped);
    connect(m_terminalPanel, &TerminalPanel::adapterReady,
        m_ctrl, &AppController::onAdapterReady);
    connect(m_ctrl, &AppController::decodedValue,
        m_terminalPanel, &TerminalPanel::onDecodedValue);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onConnected(const QString& port, int baudRate)
{
    m_commandPanel->setConnected(true);
    m_scenarioPanel->setConnected(true);
    m_statusLabel->setText("● Подключено");
    m_statusLabel->setStyleSheet("color: #3D7A52; font-weight: 600;");
    m_portLabel->setText(QString("%1  ·  %2 бод").arg(port).arg(baudRate));
    m_terminalPanel->onCommandSent(
        QString("=== Подключено к %1 (%2 бод) ===").arg(port).arg(baudRate));
}

void MainWindow::onDisconnected()
{
    m_commandPanel->setConnected(false);
    m_scenarioPanel->setConnected(false);
    m_statusLabel->setText("○ Не подключено");
    m_statusLabel->setStyleSheet("color: #8A8278;");
    m_portLabel->setText("");
    m_terminalPanel->onConnectionError("=== Отключено ===");
}

void MainWindow::onStatusMessage(const QString& message)
{
    statusBar()->showMessage(message, 5000);
}

void MainWindow::setupMenu()
{
    QMenu* fileMenu = menuBar()->addMenu("Файл");

    QAction* loadAct = fileMenu->addAction("Загрузить команды и сценарии...");
    loadAct->setShortcut(QKeySequence::Open);
    connect(loadAct, &QAction::triggered, this, &MainWindow::onLoadFile);

    QAction* saveAct = fileMenu->addAction("Сохранить команды и сценарии...");
    saveAct->setShortcut(QKeySequence::Save);
    connect(saveAct, &QAction::triggered, this, &MainWindow::onSaveFile);
}

void MainWindow::onLoadFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Загрузить файл", "", "JSON (*.json)");
    if (path.isEmpty()) return;

    if (m_ctrl->loadFromFile(path)) {
        m_commandPanel->refreshList();
        m_scenarioPanel->refreshList();
        statusBar()->showMessage(QString("Загружено: %1").arg(path), 5000);
    }
    else {
        QMessageBox::warning(this, "Ошибка", "Не удалось загрузить файл.");
    }
}

void MainWindow::onSaveFile()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "Сохранить файл", "commands.json", "JSON (*.json)");
    if (path.isEmpty()) return;

    if (m_ctrl->saveToFile(path))
        statusBar()->showMessage(QString("Сохранено: %1").arg(path), 5000);
    else
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл.");
}