#pragma once

#include <QObject>
#include <QSerialPort>
#include <QStringList>

class SerialConnection : public QObject {
    Q_OBJECT

public:
    explicit SerialConnection(QObject* parent = nullptr);
    ~SerialConnection();

    bool connectPort(const QString& portName, int baudRate);
    void disconnectPort();

    bool isOpen() const;
    bool send(const QString& data);

    static QStringList availablePorts();

signals:
    // Эмитируется каждый раз когда пришли новые данные из порта
    void dataReceived(const QString& data);

    // Эмитируется при ошибке порта
    void errorOccurred(const QString& message);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort m_serial;
};