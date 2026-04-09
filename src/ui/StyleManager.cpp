#include "StyleManager.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>

QString StyleManager::loadTheme(const QString& resourcePath) {
    QFile f(resourcePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "StyleManager: cannot open theme resource:" << resourcePath;
        return QString();
    }
    QTextStream in(&f);
    QString content = in.readAll();
    f.close();
    return content;
}

void StyleManager::applyTheme(const QString& resourcePath, QApplication* app) {
    if (!app) app = qobject_cast<QApplication*>(QCoreApplication::instance());
    if (!app) return;

    QString qss = loadTheme(resourcePath);
    if (qss.isEmpty()) return;

    app->setStyleSheet(qss);
}
