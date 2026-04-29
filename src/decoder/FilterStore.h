#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <optional>

// ─────────────────────────────────────────────────────────────────────────────
// CanFilter — описание одного правила декодирования CAN кадра.
//
// Формат входной строки: "18 FF 97 90 5 01 41 A0 00 00"
//   canId   = первые 3 байта без пробелов в верхнем регистре: "18FF97"
//   Байты данных b0..b7 подставляются в formula автоматически.
//
// Примеры формул:
//   ieee754(b0,b1,b2,b3)           — big-endian float
//   (b0 * 256.0 + b1) * 0.1        — два байта → значение с масштабом
//   b0 - 40                        — один байт со смещением
// ─────────────────────────────────────────────────────────────────────────────
struct CanFilter {
    int     id = 0;
    QString canId;      // "18FF97"  (без пробелов, верхний регистр)
    QString name;       // "Температура контроллера"
    QString unit;       // "°C"
    QString formula;    // "ieee754(b0,b1,b2,b3)"
    bool    enabled = true;

    bool isValid() const {
        return !canId.isEmpty() && !formula.isEmpty();
    }
};

// ─────────────────────────────────────────────────────────────────────────────

class FilterStore : public QObject {
    Q_OBJECT

public:
    explicit FilterStore(QObject* parent = nullptr);

    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath) const;

    void addFilter(const CanFilter& f);
    bool removeById(int id);
    bool updateFilter(const CanFilter& f);

    std::optional<CanFilter> findById(int id) const;

    // Все фильтры подходящие для данного CAN ID (могут быть несколько)
    QList<CanFilter> findByCanId(const QString& canId) const;

    const QList<CanFilter>& filters() const;

    // Создать filters.json с дефолтными фильтрами если файл не существует
    static void createDefaultFile(const QString& filePath);

private:
    QList<CanFilter> m_filters;
    int              m_nextId = 1;
    int              generateId();
};