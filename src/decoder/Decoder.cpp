#include "Decoder.h"

#include "Filter.h"
#include <QJsonDocument>
#include <QJsonObject>

Decoder::Decoder(QObject* parent)
    : QObject(parent)
{
}

const QString Decoder::decode(const QString& rawData)
{
    if (rawData.isEmpty()) {
        return QString();
    }

    if (rawData.length() < 10) {
        return rawData;
    }
    QString cleaned = rawData;
    cleaned.remove(' ');
    cleaned.remove('>');
    cleaned.remove('\r');
    cleaned.remove('\n');
    qDebug() << "Cleaned:" << cleaned;

    QString idPart = cleaned.left(8);
    QString dlcPart = cleaned.mid(8, 2);
    QString dataPart = cleaned.mid(10);

    qDebug() << "ID:" << idPart;
    qDebug() << "DLC:" << dlcPart;
    qDebug() << "Data:" << dataPart;

    if (isValidCAN(idPart, dlcPart, dataPart)) {
        qDebug() << "valid1";
        QList<int> numbers = hexToDecimalBytes(dataPart);
        if (idPart == "18FF9790" && dlcPart == "5") {
            qDebug() << "valid2";
            QList<int> numbers = hexToDecimalBytes(dataPart);
            float num_res = decodeDataFor5Nums(numbers);
            QString result = QString::number(num_res, 'f', 4);
            qDebug() << "res" << result;
            return result;
        }
    }


    return rawData;
}

bool Decoder::isValidCAN(QString id, QString dlc, QString data)
{
    if (id.length() != 8) return false;
    
    bool ok;
    int dlcValue = dlc.toInt(&ok, 16);
    if (!ok || dlcValue < 0 || dlcValue > 8) return false;

    if (data.length() != dlcValue * 2) return false;
    return true;
}

QString Decoder::decodeData(QString data)
{
    return QString();
}

QList<int> Decoder::hexToDecimalBytes(const QString& hexString)
{
    QList<int> decimalBytes;

    // Убираем все пробелы
    QString cleaned = hexString;
    cleaned.remove(' ');

    // Проходим по каждому байту (2 символа)
    for (int i = 0; i < cleaned.length(); i += 2) {
        QString byteHex = cleaned.mid(i, 2);
        bool ok;
        int byteDec = byteHex.toInt(&ok, 16);

        if (ok) {
            decimalBytes.append(byteDec);
        }
        else {
            qDebug() << "Ошибка перевода HEX:" << byteHex;
            decimalBytes.append(-1); // Ошибка
        }
    }

    return decimalBytes;
}

float Decoder::decodeDataFor5Nums(const QList<int> numbers)
{
    int implicit_bit = (numbers[1] > 127) ? 1 : 0;
    int sign = (numbers[0] > 127) ? 1 : 0;
    int mantissa = (numbers[1] > 127) ? (numbers[1] - 128) : numbers[1];
    int exp = (numbers[0] > 127 ? (numbers[0] - 128) : numbers[0]) * 2 + implicit_bit;
    int Q = numbers[2] + numbers[3] * 256 + mantissa * 256 * 256;
    float  result = (float)sign
        * (float)std::pow(2.0, exp - 127)
        * (1.0f + (float)Q / (float)(1 << 23));
    return result;
}

