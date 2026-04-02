#include "CanDecoder.h"
#include <cstring>

CanDecoder::CanDecoder(QObject* parent) : QObject(parent)
{
    initParameters();
}

bool CanDecoder::isEnabled() const { return m_enabled; }
void CanDecoder::setEnabled(bool e) { m_enabled = e; }

void CanDecoder::initParameters()
{
    auto& p97 = m_params["18FF97"];
    p97[0x01] = { "Температура контроллера" };
    p97[0x02] = { "Температура транзисторов" };
    p97[0x03] = { "Температура корпуса" };
    p97[0x04] = { "Температура окруж. среды" };
    p97[0x05] = { "Ток I1" };
    p97[0x06] = { "Ток I2" };
    p97[0x07] = { "Ток I3" };
    p97[0x08] = { "Напряжение B" };
    p97[0x09] = { "Напряжение B1" };
    p97[0x0A] = { "Напряжение B2" };
    p97[0x0B] = { "Напряжение B3" };
    p97[0x0C] = { "Напряжение G" };
    p97[0x0D] = { "dU" };
    p97[0x0E] = { "on" };
    p97[0x0F] = { "АПЖ" };

    auto& p98 = m_params["18FF98"];
    p98[0x33] = { "VCC" };
    p98[0x34] = { "REF" };
    p98[0x35] = { "RT3" };
    p98[0x36] = { "VIOUT1" };
    p98[0x37] = { "VIOUT2" };
    p98[0x38] = { "VIOUT3" };

    // 18FF93 — нет subId, одна запись
    m_params["18FF93"][0] = { "KF1 KF2 KF3 KF4" };
}

// Формат строки: "18 FF 97 90 <size> <subId> <b0> <b1> <b2> <b3>"
//                  [0] [1] [2] [3]   [4]     [5]  [6]  [7]  [8]  [9]
DecodeResult CanDecoder::decode(const QString& line) const
{
    DecodeResult result;
    if (!m_enabled) return result;

    const QStringList parts = line.trimmed().split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 5) return result;

    const QString canId = (parts[0] + parts[1] + parts[2]).toUpper();
    result.canId = canId;

    if (canId == "18FF97") return decode97(parts);
    if (canId == "18FF98") return decode98(parts);
    if (canId == "18FF93") return decode93(parts);

    return result;
}

DecodeResult CanDecoder::decode97(const QStringList& parts) const
{
    // parts: [0]=18 [1]=FF [2]=97 [3]=90 [4]=5 [5]=subId [6]=b0 [7]=b1 [8]=b2 [9]=b3
    DecodeResult r;
    if (parts.size() < 10) return r;

    r.canId = "18FF97";
    r.subId = hexByte(parts[5]);

    const uint8_t b0 = hexByte(parts[6]);
    const uint8_t b1 = hexByte(parts[7]);
    const uint8_t b2 = hexByte(parts[8]);
    const uint8_t b3 = hexByte(parts[9]);

    r.value = ieeeFloat(b0, b1, b2, b3);
    r.valid = true;
    return r;
}

DecodeResult CanDecoder::decode98(const QStringList& parts) const
{
    // parts: [0]=18 [1]=FF [2]=98 [3]=90 [4]=3 [5]=subId [6]=b0 [7]=b1
    DecodeResult r;
    if (parts.size() < 8) return r;

    r.canId = "18FF98";
    r.subId = hexByte(parts[5]);

    // Конкатенация двух байт → HEX → DEC
    const QString hexStr = parts[6].toUpper() + parts[7].toUpper();
    bool ok;
    r.value = hexStr.toInt(&ok, 16);
    if (!ok) return r;

    r.valid = true;
    return r;
}

DecodeResult CanDecoder::decode93(const QStringList& parts) const
{
    // parts: [0]=18 [1]=FF [2]=93 [3]=90 [4]=4 [5]=b0 [6]=b1 [7]=b2 [8]=b3
    DecodeResult r;
    if (parts.size() < 9) return r;

    r.canId = "18FF93";
    r.subId = -1; // нет subId

    const int v0 = hexByte(parts[5]);
    const int v1 = hexByte(parts[6]);
    const int v2 = hexByte(parts[7]);
    const int v3 = hexByte(parts[8]);

    r.text = QString("KF1=%1 KF2=%2 KF3=%3 KF4=%4").arg(v0).arg(v1).arg(v2).arg(v3);
    r.valid = true;
    return r;
}

float CanDecoder::ieeeFloat(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
    const uint32_t raw = (uint32_t(b0) << 24) | (uint32_t(b1) << 16)
        | (uint32_t(b2) << 8) | uint32_t(b3);
    float f;
    std::memcpy(&f, &raw, sizeof(float));
    return f;
}

uint8_t CanDecoder::hexByte(const QString& hex)
{
    return static_cast<uint8_t>(hex.toUInt(nullptr, 16));
}

void CanDecoder::setParameter(const QString& canId, int subId, const CanParameter& param)
{
    m_params[canId.toUpper()][subId] = param;
}

void CanDecoder::removeParameter(const QString& canId, int subId)
{
    if (m_params.contains(canId.toUpper()))
        m_params[canId.toUpper()].remove(subId);
}