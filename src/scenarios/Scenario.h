#pragma once

#include <QString>
#include <QStringList>

// Сценарий — именованная последовательность команд с паузой между ними.
// steps хранят текст команд напрямую (не ID) — одна команда может
// встречаться несколько раз, и сценарий самодостаточен без привязки к Store.
struct Scenario {
    int         id;
    QString     name;
    QString     description;
    int         delayMs = 1000;   // пауза между командами в мс
    QStringList steps;            // ["ATZ", "ATH1", "ATD1", ...]

    bool isValid() const {
        return !name.isEmpty() && !steps.isEmpty();
    }
};