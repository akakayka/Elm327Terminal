#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include "FilterStore.h"
#include "ExprEval.h"

struct DecodeResult {
    bool    valid = false;
    QString canId;
    QString name;
    QString unit;
    double  value = 0.0;
    QString error;          // если формула не вычислилась
    QString formatted;      // готовая строка для вывода
};

// CanDecoder — разбирает CAN строку и применяет подходящие фильтры из FilterStore.
// Для каждого совпадения по canId вычисляет формулу через ExprEval.
class CanDecoder : public QObject {
    Q_OBJECT

public:
    explicit CanDecoder(FilterStore* store, QObject* parent = nullptr);

    // Попытаться декодировать строку из терминала.
    // Возвращает список результатов (один canId может иметь несколько фильтров).
    QList<DecodeResult> decode(const QString& line) const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

private:
    // Парсим строку → canId + байты данных
    struct ParsedFrame {
        bool        valid = false;
        QString     canId;
        QList<uint8_t> bytes; // все байты включая subId
    };

    static ParsedFrame parseFrame(const QString& line);
    static uint8_t     hexByte(const QString& s);

    FilterStore* m_store;
    bool         m_enabled = true;
};