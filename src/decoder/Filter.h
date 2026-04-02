#pragma once

#include <QString>

struct Filter {
    int     id;
    QString triggerHex;
    std::function<QString(const QString&)> decoder;
};