#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QStringList>

struct CanParameter {
    QString name;
};

struct DecodeResult {
    bool    valid = false;
    QString canId;
    int     subId = -1;   // -1 если нет subId (18FF93)
    double  value = 0;
    QString text;          // для 18FF93 — строка всех значений
};

class CanDecoder : public QObject {
    Q_OBJECT

public:
    explicit CanDecoder(QObject* parent = nullptr);

    DecodeResult decode(const QString& line) const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    void setParameter(const QString& canId, int subId, const CanParameter& param);
    void removeParameter(const QString& canId, int subId);

    const QMap<QString, QMap<int, CanParameter>>& parameters() const
    {
        return m_params;
    }

private:
    void initParameters();

    DecodeResult decode97(const QStringList& parts) const;
    DecodeResult decode98(const QStringList& parts) const;
    DecodeResult decode93(const QStringList& parts) const;

    static float   ieeeFloat(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
    static uint8_t hexByte(const QString& hex);

    QMap<QString, QMap<int, CanParameter>> m_params;
    bool m_enabled = true;
};