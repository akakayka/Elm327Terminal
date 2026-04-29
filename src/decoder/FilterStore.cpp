#include "FilterStore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

FilterStore::FilterStore(QObject* parent)
    : QObject(parent)
{
}

// ── JSON ──────────────────────────────────────────────────────────────────────

bool FilterStore::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return false;

    m_filters.clear();
    m_nextId = 1;

    const QJsonArray arr = doc.object().value("filters").toArray();
    for (const QJsonValue& v : arr) {
        QJsonObject o = v.toObject();
        CanFilter f;
        f.id = o.value("id").toInt();
        f.canId = o.value("canId").toString().toUpper().remove(' ');
        f.name = o.value("name").toString();
        f.unit = o.value("unit").toString();
        f.formula = o.value("formula").toString();
        f.enabled = o.value("enabled").toBool(true);

        if (f.isValid()) {
            m_filters.append(f);
            if (f.id >= m_nextId) m_nextId = f.id + 1;
        }
    }
    return true;
}

bool FilterStore::saveToFile(const QString& filePath) const
{
    QJsonArray arr;
    for (const CanFilter& f : m_filters) {
        QJsonObject o;
        o["id"] = f.id;
        o["canId"] = f.canId;
        o["name"] = f.name;
        o["unit"] = f.unit;
        o["formula"] = f.formula;
        o["enabled"] = f.enabled;
        arr.append(o);
    }
    QJsonObject root;
    root["filters"] = arr;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

// ── CRUD ──────────────────────────────────────────────────────────────────────

void FilterStore::addFilter(const CanFilter& f)
{
    CanFilter copy = f;
    copy.id = generateId();
    m_filters.append(copy);
}

bool FilterStore::removeById(int id)
{
    for (int i = 0; i < m_filters.size(); ++i) {
        if (m_filters[i].id == id) { m_filters.removeAt(i); return true; }
    }
    return false;
}

bool FilterStore::updateFilter(const CanFilter& f)
{
    for (CanFilter& existing : m_filters) {
        if (existing.id == f.id) { existing = f; return true; }
    }
    return false;
}

std::optional<CanFilter> FilterStore::findById(int id) const
{
    for (const CanFilter& f : m_filters)
        if (f.id == id) return f;
    return std::nullopt;
}

QList<CanFilter> FilterStore::findByCanId(const QString& canId) const
{
    QList<CanFilter> result;
    const QString upper = canId.toUpper();
    for (const CanFilter& f : m_filters)
        if (f.enabled && f.canId == upper) result.append(f);
    return result;
}

const QList<CanFilter>& FilterStore::filters() const { return m_filters; }

int FilterStore::generateId() { return m_nextId++; }

// ── Дефолтный файл ────────────────────────────────────────────────────────────

void FilterStore::createDefaultFile(const QString& filePath)
{
    // Переносим все встроенные фильтры из CanDecoder в JSON
    QJsonArray arr;

    auto add = [&](int id, const QString& canId, const QString& name,
        const QString& unit, const QString& formula) {
            QJsonObject o;
            o["id"] = id;
            o["canId"] = canId;
            o["name"] = name;
            o["unit"] = unit;
            o["formula"] = formula;
            o["enabled"] = true;
            arr.append(o);
        };

    // 18FF97 — IEEE 754 float, байт 0 (после subId) — данные с позиции b0
    // Строка: "18 FF 97 90 5 01 41 A0 00 00"
    // subId = b0 (первый байт данных), float = b1..b4
    // Поэтому формула для 18FF97: ieee754(b1,b2,b3,b4)
    add(1, "18FF97", "Температура контроллера", "°C", "ieee754(b1,b2,b3,b4)");
    add(2, "18FF98", "Аналоговые каналы", "у.е.", "hex2dec(b1,b2)");
    add(3, "18FF93", "Коэффициенты KF", "DEC", "b1");

    QJsonObject root;
    root["filters"] = arr;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}