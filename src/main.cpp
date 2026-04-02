#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "controllers/AppController.h"
#include "ui/MainWindow.h"

// Создаём файл с базовыми командами и сценарием
static void createDefaultFile(const QString& path)
{
    QJsonArray commands;
    auto addCmd = [&](int id, const QString& text, const QString& name) {
        QJsonObject o;
        o["id"] = id;
        o["name"] = name;
        o["text"] = text;
        commands.append(o);
        };

    addCmd(1, "ATZ", "Сброс адаптера");
    addCmd(2, "ATI", "Информация об адаптере");
    addCmd(3, "AT@1", "Информация об устройстве");
    addCmd(4, "AT@2", "Идентификатор устройства");
    addCmd(5, "ATH1", "Включить заголовки");
    addCmd(6, "ATH0", "Выключить заголовки");
    addCmd(7, "ATE0", "Выключить эхо");
    addCmd(8, "ATE1", "Включить эхо");
    addCmd(9, "ATL0", "Выключить светодиод");
    addCmd(10, "ATL1", "Включить светодиод");
    addCmd(11, "ATSP0", "Автоматический выбор протокола");
    addCmd(12, "ATDP", "Опрос текущего протокола");
    addCmd(13, "ATD1", "Включение отображения длины данных (DLC)");
    addCmd(14, "ATAL", "Работа с полными 8-байтными CAN-фреймами");
    addCmd(15, "ATSP9", "CAN 29bit 250Kbps");
    addCmd(16, "ATCAF0", "Отключение автоформатирования");
    addCmd(17, "ATCF7FF", "Установить фильтр на все сообщения");
    addCmd(18, "ATCM000", "Маска для приема всех сообщений");
    addCmd(19, "ATCP00", "Установка первых бит ID");
    addCmd(20, "ATSH000001", "Установка iD 01");

    // Базовый сценарий — шаги по тексту команд
    QJsonObject scenario;
    scenario["id"] = 1;
    scenario["name"] = "Базовая настройка CAN";
    scenario["description"] = "Базовая настройка CAN";
    scenario["delayMs"] = 500;
    scenario["steps"] = QJsonArray{
        "ATZ", "ATH1", "ATD1", "ATAL",
        "ATSP9", "ATCAF0", "ATCF7FF",
        "ATCM000", "ATCP00", "ATSH000001"
    };

    QJsonObject root;
    root["commands"] = commands;
    root["scenarios"] = QJsonArray{ scenario };

    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ELM327 Commander");

    AppController controller;

    const QString defaultPath =
        QDir(QApplication::applicationDirPath()).filePath("commands.json");

    if (!QFile::exists(defaultPath))
        createDefaultFile(defaultPath);

    controller.loadFromFile(defaultPath);

    MainWindow window(&controller);
    window.show();

    const int result = app.exec();
    controller.saveToFile(defaultPath);
    return result;
}