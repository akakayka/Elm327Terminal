#pragma once

#include <QObject>
#include <QString>
#include <optional>
#include <QList>

class Decoder : public QObject {
    Q_OBJECT
    public:
        explicit Decoder(QObject* parent = nullptr);
        const QString decode(const QString& name);
    private:
        bool isValidCAN(QString id, QString dlc, QString data);
        QString decodeData(QString data);
        QList<int> hexToDecimalBytes(const QString& hexString);
        float decodeDataFor5Nums(const QList<int> data);
};