#include "ConnectionPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

#include "../serial/SerialConnection.h"
#include "../serial/AutoConnectWorker.h"

static const QList<int> BAUD_RATES = {
    9600, 19200, 38400, 57600, 115200
};

ConnectionPanel::ConnectionPanel(AppController* controller, QWidget* parent)
    : QWidget(parent)
    , m_ctrl(controller)
{
    setupUi();
    onRefreshClicked();
}

ConnectionPanel::~ConnectionPanel()
{
    // Если поток ещё работает — дождёмся завершения
    if (m_workerThread && m_workerThread->isRunning()) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

// ── UI ────────────────────────────────────────────────────────────────────────

void ConnectionPanel::setupUi()
{
    m_portCombo = new QComboBox(this);
    m_baudCombo = new QComboBox(this);
    m_refreshBtn = new QPushButton(" Обновить", this);
    m_refreshBtn->setIcon(QIcon(":/style/icons/ref.svg")); m_refreshBtn->setStyleSheet(R"(
        QPushButton {
            padding-left: 10px;
        }
        QPushButton::icon {
            left: 4px;
            position: absolute;
            
        }
    )");
    m_refreshBtn->setIconSize(QSize(14, 14));
    m_connectBtn = new QPushButton("Подключиться", this);
    m_disconnectBtn = new QPushButton("Отключиться", this);
    m_autoConnectBtn = new QPushButton("Авто", this);

    m_connectBtn->setProperty("accent", true);
    m_disconnectBtn->setProperty("danger", true);

    // Компактные размеры
    m_portCombo->setMinimumWidth(70);
    m_baudCombo->setFixedWidth(100);
    m_refreshBtn->setFixedWidth(106);
    m_refreshBtn->setToolTip("Обновить список портов");
    m_autoConnectBtn->setFixedWidth(60);
    m_autoConnectBtn->setToolTip("Автоматическое подключение");

    for (int baud : BAUD_RATES)
        m_baudCombo->addItem(QString::number(baud), baud);
    m_baudCombo->setCurrentIndex(2);

    // ── Всё в одну строку ─────────────────────────────────────────────────────
    QHBoxLayout* row = new QHBoxLayout;
    row->setSpacing(6);
    row->addWidget(new QLabel("Порт:", this));
    row->addWidget(m_portCombo, 1);
    row->addWidget(m_refreshBtn);
    row->addSpacing(8);
    row->addWidget(new QLabel("Скорость:", this));
    row->addWidget(m_baudCombo);
    row->addSpacing(8);
    row->addWidget(m_connectBtn);
    row->addWidget(m_disconnectBtn);
    row->addWidget(m_autoConnectBtn);

    QGroupBox* group = new QGroupBox(this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    groupLayout->setContentsMargins(10, 10, 10, 10);
    groupLayout->addLayout(row);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(group);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    connect(m_connectBtn, &QPushButton::clicked, this, &ConnectionPanel::onConnectClicked);
    connect(m_disconnectBtn, &QPushButton::clicked, this, &ConnectionPanel::onDisconnectClicked);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ConnectionPanel::onRefreshClicked);
    connect(m_autoConnectBtn, &QPushButton::clicked, this, &ConnectionPanel::onAutoConnectClicked);

    updateButtonStates(false);
}

void ConnectionPanel::updateButtonStates(bool isConnected)
{
    m_connectBtn->setEnabled(!isConnected);
    m_disconnectBtn->setEnabled(isConnected);
    m_portCombo->setEnabled(!isConnected);
    m_baudCombo->setEnabled(!isConnected);
    m_refreshBtn->setEnabled(!isConnected);
    m_autoConnectBtn->setEnabled(!isConnected);
}

void ConnectionPanel::setAutoConnecting(bool active)
{
    // Во время поиска блокируем все кнопки кроме ничего — просто ждём
    m_connectBtn->setEnabled(!active);
    m_disconnectBtn->setEnabled(!active);
    m_refreshBtn->setEnabled(!active);
    m_autoConnectBtn->setEnabled(!active);
    m_portCombo->setEnabled(!active);
    m_baudCombo->setEnabled(!active);

    m_autoConnectBtn->setText(active ? "..." : "Авто");
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void ConnectionPanel::onRefreshClicked()
{
    const QString current = m_portCombo->currentText();
    m_portCombo->clear();

    const QStringList ports = SerialConnection::availablePorts();
    if (ports.isEmpty()) {
        m_portCombo->addItem("— нет портов —");
        emit statusMessage("Доступные порты не найдены");
        return;
    }

    m_portCombo->addItems(ports);

    int idx = m_portCombo->findText(current);
    if (idx >= 0)
        m_portCombo->setCurrentIndex(idx);
}

void ConnectionPanel::onConnectClicked()
{
    const QString port = m_portCombo->currentText();
    const int     baud = m_baudCombo->currentData().toInt();

    if (port.isEmpty() || port.startsWith("—")) {
        emit statusMessage("Выберите порт");
        return;
    }

    if (m_ctrl->connectSerial(port, baud)) {
        updateButtonStates(true);
        emit connected(port, baud);
    }
    else {
        emit statusMessage(QString("Не удалось подключиться к %1").arg(port));
        QMessageBox::warning(this, "Ошибка подключения",
            QString("Не удалось открыть порт %1.\n"
                "Проверьте, что устройство подключено и порт не занят.").arg(port));
    }
}

void ConnectionPanel::onDisconnectClicked()
{
    m_ctrl->disconnectSerial();
    updateButtonStates(false);
    emit disconnected();
    emit statusMessage("Отключено");
}

void ConnectionPanel::onAutoConnectClicked()
{
    const QStringList ports = SerialConnection::availablePorts();
    if (ports.isEmpty()) {
        emit statusMessage("Нет доступных портов");
        return;
    }

    setAutoConnecting(true);
    emit statusMessage("Автоподключение: поиск ELM327...");

    // Создаём worker и поток
    m_workerThread = new QThread(this);
    auto* worker = new AutoConnectWorker(ports, BAUD_RATES);
    worker->moveToThread(m_workerThread);

    // Запуск
    connect(m_workerThread, &QThread::started, worker, &AutoConnectWorker::run);

    // Результаты
    connect(worker, &AutoConnectWorker::found,
        this, &ConnectionPanel::onAutoConnectFound);
    connect(worker, &AutoConnectWorker::notFound,
        this, &ConnectionPanel::onAutoConnectNotFound);
    connect(worker, &AutoConnectWorker::progress,
        this, &ConnectionPanel::onAutoConnectProgress);

    // Очистка: когда worker завершил — останавливаем поток и удаляем объекты
    connect(worker, &AutoConnectWorker::finished,
        m_workerThread, &QThread::quit);
    connect(m_workerThread, &QThread::finished,
        worker, &QObject::deleteLater);
    connect(m_workerThread, &QThread::finished,
        m_workerThread, &QObject::deleteLater);

    m_workerThread->start();
}

// ── Результаты автоподключения ────────────────────────────────────────────────

void ConnectionPanel::onAutoConnectFound(const QString& port, int baudRate)
{
    // Worker нашёл ELM327 — теперь подключаемся через AppController
    if (m_ctrl->connectSerial(port, baudRate)) {
        // Обновляем комбобоксы чтобы показывали найденный порт/скорость
        int portIdx = m_portCombo->findText(port);
        if (portIdx >= 0) m_portCombo->setCurrentIndex(portIdx);

        int baudIdx = m_baudCombo->findData(baudRate);
        if (baudIdx >= 0) m_baudCombo->setCurrentIndex(baudIdx);

        setAutoConnecting(false);
        updateButtonStates(true);
        emit connected(port, baudRate);
        emit statusMessage(QString("Автоподключение: найден ELM327 на %1 @ %2").arg(port).arg(baudRate));
    }
    else {
        // Порт нашли но подключиться не смогли (редкий случай — порт занят)
        setAutoConnecting(false);
        updateButtonStates(false);
        emit statusMessage(QString("Найден %1, но не удалось подключиться").arg(port));
    }
}

void ConnectionPanel::onAutoConnectNotFound()
{
    setAutoConnecting(false);
    updateButtonStates(false);
    emit statusMessage("Автоподключение: ELM327 не найден");
    QMessageBox::information(this, "Автоподключение",
        "Устройство ELM327 не найдено ни на одном порту.\n"
        "Проверьте подключение адаптера.");
}

void ConnectionPanel::onAutoConnectProgress(const QString& message)
{
    emit statusMessage(message);
}