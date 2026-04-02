#pragma once

#include <QObject>
#include <QList>
#include <QJsonObject>
#include <optional>
#include "Scenario.h"

class ScenarioStore : public QObject {
    Q_OBJECT

public:
    explicit ScenarioStore(QObject* parent = nullptr);

    // Загрузка/сохранение — работает с тем же JSON что и CommandStore
    bool loadFromJson(const QJsonObject& root);
    void saveToJson(QJsonObject& root) const;

    // CRUD
    void addScenario(const Scenario& s);
    bool removeById(int id);
    bool updateScenario(const Scenario& s);

    std::optional<Scenario> findById(int id) const;
    const QList<Scenario>& scenarios() const;

private:
    QList<Scenario> m_scenarios;
    int             m_nextId = 1;
    int             generateId();
};