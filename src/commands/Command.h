#pragma once

#include <QString>

struct Command {
    int     id;    // уникальный ID для поддержки сценариев в будущем
    QString name;  // человекочитаемое название, например "Reset adapter"
    QString text;  // строка команды, например "ATZ"

    bool isValid() const {
        return !name.isEmpty() && !text.isEmpty();
    }
};
