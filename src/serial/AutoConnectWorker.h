#pragma once

#include <QObject>
#include <QStringList>

class AutoConnectWorker : public QObject {
    Q_OBJECT

public:
    explicit AutoConnectWorker(const QStringList& ports,
        const QList<int>& baudRates,
        QObject* parent = nullptr);

public slots:
    void run();

signals:
    void found(const QString& port, int baudRate);
    void progress(const QString& message);
    void notFound();
    void finished();

private:
    QStringList m_ports;
    QList<int>  m_baudRates;
};