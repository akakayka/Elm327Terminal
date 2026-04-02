#include "AutoConnectWorker.h"

#include <QSerialPort>
#include <QThread>
#include <QDebug>

AutoConnectWorker::AutoConnectWorker(const QStringList& ports,
    const QList<int>& baudRates,
    QObject* parent)
    : QObject(parent)
    , m_ports(ports)
    , m_baudRates(baudRates)
{
}

// Считаем ответ положительным если:
// - содержит "ELM327" (полный ответ на ATI)
// - содержит эхо "ATI"  (адаптер слышит нас, эхо включено)
static bool isElmResponse(const QString& response)
{
    return response.contains("ELM327", Qt::CaseInsensitive)
        || response.contains("ATI", Qt::CaseSensitive);
}

void AutoConnectWorker::run()
{
    for (const QString& port : m_ports) {
        for (int baud : m_baudRates) {

            emit progress(QString("Проверяю %1 @ %2...").arg(port).arg(baud));

            QSerialPort serial;
            serial.setPortName(port);
            serial.setBaudRate(baud);
            serial.setDataBits(QSerialPort::Data8);
            serial.setParity(QSerialPort::NoParity);
            serial.setStopBits(QSerialPort::OneStop);
            serial.setFlowControl(QSerialPort::NoFlowControl);

            if (!serial.open(QIODevice::ReadWrite)) {
                qDebug() << "[AutoConnect] Не удалось открыть" << port << baud;
                continue;
            }

            // Короткая пауза — только чтобы порт стабилизировался
            QThread::msleep(100);
            serial.clear();

            serial.write("ATI\r");
            serial.flush();

            // Читаем ответ — максимум 1.5 сек, выходим раньше при '>'
            QString response;
            for (int i = 0; i < 10; ++i) {
                if (serial.waitForReadyRead(150))
                    response += QString::fromLatin1(serial.readAll());

                if (response.contains('>') || response.contains("ELM327", Qt::CaseInsensitive))
                    break;
            }

            serial.close();

            const QString debugStr = QString(response)
                .replace('\r', "\\r").replace('\n', "\\n");
            qDebug() << "[AutoConnect]" << port << baud << "->" << debugStr;


            if (isElmResponse(response)) {
                emit found(port, baud);
                emit finished();
                return;
            }
        }
    }

    emit notFound();
    emit finished();
}