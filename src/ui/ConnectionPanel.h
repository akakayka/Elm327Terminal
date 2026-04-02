#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QThread>

#include "../controllers/AppController.h"

class ConnectionPanel : public QWidget {
    Q_OBJECT

public:
    explicit ConnectionPanel(AppController* controller, QWidget* parent = nullptr);
    ~ConnectionPanel();

signals:
    void connected(const QString& port, int baudRate);
    void disconnected();
    void statusMessage(const QString& message);
    // Отладочная информация автоподключения → терминал
    void debugInfo(const QString& message);

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onRefreshClicked();
    void onAutoConnectClicked();

    // Результаты AutoConnectWorker
    void onAutoConnectFound(const QString& port, int baudRate);
    void onAutoConnectNotFound();
    void onAutoConnectProgress(const QString& message);

private:
    void setupUi();
    void updateButtonStates(bool isConnected);
    void setAutoConnecting(bool active); // блокирует UI во время поиска

    AppController* m_ctrl;
    QThread* m_workerThread = nullptr;

    QComboBox* m_portCombo;
    QComboBox* m_baudCombo;
    QPushButton* m_refreshBtn;
    QPushButton* m_connectBtn;
    QPushButton* m_disconnectBtn;
    QPushButton* m_autoConnectBtn;
};