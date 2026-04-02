#include "ScenarioStore.h"
#include <QJsonArray>
#include <QJsonObject>

ScenarioStore::ScenarioStore(QObject* parent)
    : QObject(parent)
{
}

// ── JSON ──────────────────────────────────────────────────────────────────────

bool ScenarioStore::loadFromJson(const QJsonObject& root)
{
    m_scenarios.clear();
    m_nextId = 1;

    const QJsonArray arr = root.value("scenarios").toArray();
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        Scenario s;
        s.id = obj.value("id").toInt();
        s.name = obj.value("name").toString();
        s.description = obj.value("description").toString();
        s.delayMs = obj.value("delayMs").toInt(1000);

        const QJsonArray steps = obj.value("steps").toArray();
        for (const QJsonValue& step : steps)
            s.steps << step.toString();

        if (s.isValid()) {
            m_scenarios.append(s);
            if (s.id >= m_nextId)
                m_nextId = s.id + 1;
        }
    }
    return true;
}

void ScenarioStore::saveToJson(QJsonObject& root) const
{
    QJsonArray arr;
    for (const Scenario& s : m_scenarios) {
        QJsonObject obj;
        obj["id"] = s.id;
        obj["name"] = s.name;
        obj["description"] = s.description;
        obj["delayMs"] = s.delayMs;

        QJsonArray steps;
        for (const QString& step : s.steps)
            steps.append(step);
        obj["steps"] = steps;

        arr.append(obj);
    }
    root["scenarios"] = arr;
}

// ── CRUD ──────────────────────────────────────────────────────────────────────

void ScenarioStore::addScenario(const Scenario& s)
{
    Scenario copy = s;
    copy.id = generateId();
    m_scenarios.append(copy);
}

bool ScenarioStore::removeById(int id)
{
    for (int i = 0; i < m_scenarios.size(); ++i) {
        if (m_scenarios[i].id == id) {
            m_scenarios.removeAt(i);
            return true;
        }
    }
    return false;
}

bool ScenarioStore::updateScenario(const Scenario& s)
{
    for (Scenario& existing : m_scenarios) {
        if (existing.id == s.id) {
            existing = s;
            return true;
        }
    }
    return false;
}

std::optional<Scenario> ScenarioStore::findById(int id) const
{
    for (const Scenario& s : m_scenarios)
        if (s.id == id) return s;
    return std::nullopt;
}

const QList<Scenario>& ScenarioStore::scenarios() const
{
    return m_scenarios;
}

int ScenarioStore::generateId()
{
    return m_nextId++;
}