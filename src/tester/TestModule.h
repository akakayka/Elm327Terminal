#pragma once

#include <QObject>
#include <vector>
#include <optional>
#include <QString>
#include <QJsonObject>
#include "Test.h"

class TestModule : public QObject {
    Q_OBJECT
public:
    explicit TestModule(QObject* parent = nullptr);

    // Загрузка/сохранение конфигурации тестов
    bool loadFromJson(const QJsonObject& root);
    void saveToJson(QJsonObject& root) const;

    // CRUD
    void addTest(const Test& t);
    bool removeById(int id);
    bool updateTest(const Test& t);

    std::optional<Test> findByCanId(int canId) const;
    const std::vector<Test>& tests() const;

    // Выполнить тест по canId, вернуть true если пройден.
    // values — расшифрованные данные (целые), при провале optionalReason содержит описание.
    bool runTestByCanId(int canId, const std::vector<int>& values, QString* optionalReason = nullptr) const;

private:
    std::vector<Test> m_tests;
    int m_nextId = 1;
    int generateId();
};
