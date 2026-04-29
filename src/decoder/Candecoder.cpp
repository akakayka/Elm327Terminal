#include "CanDecoder.h"

CanDecoder::CanDecoder(FilterStore* store, QObject* parent)
    : QObject(parent)
    , m_store(store)
{
}

bool CanDecoder::isEnabled() const { return m_enabled; }
void CanDecoder::setEnabled(bool e) { m_enabled = e; }

// ── Публичный метод ───────────────────────────────────────────────────────────

QList<DecodeResult> CanDecoder::decode(const QString& line) const
{
    QList<DecodeResult> results;
    if (!m_enabled || !m_store) return results;

    const ParsedFrame frame = parseFrame(line);
    if (!frame.valid) return results;

    const QList<CanFilter> filters = m_store->findByCanId(frame.canId);
    if (filters.isEmpty()) return results;

    for (const CanFilter& filter : filters) {
        DecodeResult r;
        r.canId = frame.canId;
        r.name = filter.name;
        r.unit = filter.unit;

        // Подставляем байты b0..b7 в вычислитель
        ExprEval eval;
        for (int i = 0; i < 8; ++i) {
            const double val = (i < frame.bytes.size()) ? frame.bytes[i] : 0.0;
            eval.setVar(QString("b%1").arg(i), val);
        }
        // Дополнительно: n = количество байт данных
        eval.setVar("n", frame.bytes.size());

        QString err;
        r.value = eval.eval(filter.formula, &err);

        if (!err.isEmpty()) {
            r.error = err;
            r.formatted = QString("    ↳ %1: [ошибка формулы: %2]").arg(r.name).arg(err);
        }
        else {
            // Форматируем число: убираем лишние нули после запятой
            const QString valStr = (r.value == std::floor(r.value) &&
                std::abs(r.value) < 1e9)
                ? QString::number((long long)r.value)
                : QString::number(r.value, 'f', 3);

            r.formatted = QString("    ↳ %1: %2%3")
                .arg(r.name)
                .arg(valStr)
                .arg(r.unit.isEmpty() ? "" : " " + r.unit);
        }
        r.valid = true;
        results.append(r);
    }
    return results;
}

// ── Парсинг ───────────────────────────────────────────────────────────────────

CanDecoder::ParsedFrame CanDecoder::parseFrame(const QString& line)
{
    ParsedFrame frame;

    // Формат: "18 FF 97 90 5 01 41 A0 00 00"
    //         [0] [1] [2][3][4][5..] данные
    const QStringList parts = line.trimmed().split(' ', Qt::SkipEmptyParts);
    if (parts.size() < 5) return frame;

    // Проверяем что первые 3 части — hex байты (составляют CAN ID)
    bool ok0, ok1, ok2;
    parts[0].toUInt(&ok0, 16);
    parts[1].toUInt(&ok1, 16);
    parts[2].toUInt(&ok2, 16);
    if (!ok0 || !ok1 || !ok2) return frame;

    frame.canId = (parts[0] + parts[1] + parts[2]).toUpper();

    // parts[3] — доп. байт (90 и т.д.), parts[4] — размер данных
    bool okSize;
    const int dataSize = parts[4].toInt(&okSize);
    if (!okSize || dataSize < 1) return frame;

    // Байты данных начинаются с parts[5]
    for (int i = 5; i < parts.size(); ++i)
        frame.bytes.append(hexByte(parts[i]));

    frame.valid = true;
    return frame;
}

uint8_t CanDecoder::hexByte(const QString& s)
{
    return static_cast<uint8_t>(s.toUInt(nullptr, 16));
}