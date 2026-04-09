#pragma once

#include <QString>

class QApplication;

class StyleManager {
public:
    static QString loadTheme(const QString& resourcePath);
    static void applyTheme(const QString& resourcePath, QApplication* app = nullptr);
};
