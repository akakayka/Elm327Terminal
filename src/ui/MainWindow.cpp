#include "MainWindow.h"

#include <QVBoxLayout>
#include <QWidget>
#include <QStatusBar>
#include <QSplitter>
#include <QTabWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(AppController* controller, QWidget* parent)
    : QMainWindow(parent)
    , m_ctrl(controller)
{
    setWindowTitle("ELM327 Commander");
    setMinimumSize(900, 650);
    setupMenu();
    setupUi();
}

void MainWindow::setupUi()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(4);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ── Панель подключения вверху ─────────────────────────────────────────────
    m_connectionPanel = new ConnectionPanel(m_ctrl, this);
    mainLayout->addWidget(m_connectionPanel);

    // ── Сплиттер: вкладки слева, терминал справа ─────────────────────────────
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);

    // Вкладки: Команды / Сценарии
    QTabWidget* tabs = new QTabWidget(this);
    m_commandPanel = new CommandPanel(m_ctrl, this);
    m_scenarioPanel = new ScenarioPanel(m_ctrl, this);
    m_decoderPanel = new DecoderPanel(m_ctrl, this);
    m_decoderPanel = new DecoderPanel(m_ctrl, this);
    tabs->addTab(m_commandPanel, "Команды");
    tabs->addTab(m_scenarioPanel, "Сценарии");
    tabs->addTab(m_decoderPanel, "Декодер");

    m_terminalPanel = new TerminalPanel(this);

    splitter->addWidget(tabs);
    splitter->addWidget(m_terminalPanel);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);

    mainLayout->addWidget(splitter, 1);

    // ── Статусбар ─────────────────────────────────────────────────────────────
    m_statusLabel = new QLabel("Не подключено", this);
    m_portLabel = new QLabel("", this);
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_portLabel);

    // ── Сигналы: ConnectionPanel ──────────────────────────────────────────────
    connect(m_connectionPanel, &ConnectionPanel::connected,
        this, &MainWindow::onConnected);
    connect(m_connectionPanel, &ConnectionPanel::disconnected,
        this, &MainWindow::onDisconnected);
    connect(m_connectionPanel, &ConnectionPanel::statusMessage,
        this, &MainWindow::onStatusMessage);

    // ── Сигналы: CommandPanel ─────────────────────────────────────────────────
    connect(m_commandPanel, &CommandPanel::statusMessage,
        this, &MainWindow::onStatusMessage);

    // ── Сигналы: ScenarioPanel ────────────────────────────────────────────────
    connect(m_scenarioPanel, &ScenarioPanel::statusMessage,
        this, &MainWindow::onStatusMessage);
    connect(m_decoderPanel, &DecoderPanel::statusMessage,
        this, &MainWindow::onStatusMessage);

    // ── Сигналы: AppController → TerminalPanel ────────────────────────────────
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
    // TerminalPanel видит '>' → сообщает AppController → ScenarioRunner
    connect(m_terminalPanel, &TerminalPanel::adapterReady,
        m_ctrl, &AppController::onAdapterReady);
    // Декодированные значения
    connect(m_ctrl, &AppController::decodedValue,
        m_terminalPanel, &TerminalPanel::onDecodedValue);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onConnected(const QString& port, int baudRate)
{
    m_commandPanel->setConnected(true);
    m_scenarioPanel->setConnected(true);
    m_statusLabel->setText("Подключено");
    m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    m_portLabel->setText(QString("Порт: %1  Скорость: %2").arg(port).arg(baudRate));
    m_terminalPanel->onCommandSent(
        QString("=== Подключено к %1 (%2 бод) ===").arg(port).arg(baudRate));
}

void MainWindow::onDisconnected()
{
    m_commandPanel->setConnected(false);
    m_scenarioPanel->setConnected(false);
    m_statusLabel->setText("Не подключено");
    m_statusLabel->setStyleSheet("");
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
        // Обновляем оба списка
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