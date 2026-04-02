#include "SerialConnection.h"
#include <QSerialPortInfo>

SerialConnection::SerialConnection(QObject* parent)
    : QObject(parent)
{
    connect(&m_serial, &QSerialPort::readyRead,
        this, &SerialConnection::onReadyRead);

    connect(&m_serial, &QSerialPort::errorOccurred,
        this, &SerialConnection::onErrorOccurred);
}

SerialConnection::~SerialConnection()
{
    disconnectPort();
}

bool SerialConnection::connectPort(const QString& portName, int baudRate)
{
    if (m_serial.isOpen())
        m_serial.close();

    m_serial.setPortName(portName);
    m_serial.setBaudRate(baudRate);
    m_serial.setDataBits(QSerialPort::Data8);
    m_serial.setParity(QSerialPort::NoParity);
    m_serial.setStopBits(QSerialPort::OneStop);
    m_serial.setFlowControl(QSerialPort::NoFlowControl);

    return m_serial.open(QIODevice::ReadWrite);
}

void SerialConnection::disconnectPort()
{
    if (m_serial.isOpen())
        m_serial.close();
}

bool SerialConnection::isOpen() const
{
    return m_serial.isOpen();
}

bool SerialConnection::send(const QString& data)
{
    if (!m_serial.isOpen())
        return false;

    QByteArray bytes = (data + "\r").toUtf8();
    const bool ok = m_serial.write(bytes) == bytes.size();
    if (ok)
        m_serial.flush(); // гарантируем что данные ушли в порт немедленно
    return ok;
}

QStringList SerialConnection::availablePorts()
{
    QStringList ports;
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts())
        ports << info.portName();
    return ports;
}

// ── Private slots ─────────────────────────────────────────────────────────────

void SerialConnection::onReadyRead()
{
    const QString data = QString::fromLatin1(m_serial.readAll());
    if (!data.isEmpty())
        emit dataReceived(data);
}

void SerialConnection::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    emit errorOccurred(m_serial.errorString());
}