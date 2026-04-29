#include "TestModule.h"
#include <QJsonArray>
#include <QJsonObject>

TestModule::TestModule(QObject* parent)
    : QObject(parent)
{
}

bool TestModule::loadFromJson(const QJsonObject& root)
{
    m_tests.clear();
    m_nextId = 1;

    const QJsonArray arr = root.value("tests").toArray();
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        Test t;
        t.id = obj.value("id").toInt();
        t.name = obj.value("name").toString();
        t.description = obj.value("description").toString();
        t.canId = obj.value("canId").toInt();

        QJsonArray ranges = obj.value("ranges").toArray();
        for (const QJsonValue& r : ranges) {
            QJsonObject ro = r.toObject();
            Range range;
            range.min = ro.value("min").toInt();
            range.max = ro.value("max").toInt();
            t.ranges.push_back(range);
        }

        if (t.isValid()) {
            m_tests.push_back(t);
            if (t.id >= m_nextId) m_nextId = t.id + 1;
        }
    }
    return true;
}

void TestModule::saveToJson(QJsonObject& root) const
{
    QJsonArray arr;
    for (const Test& t : m_tests) {
        QJsonObject obj;
        obj["id"] = t.id;
        obj["name"] = t.name;
        obj["description"] = t.description;
        obj["canId"] = t.canId;
        QJsonArray ranges;
        for (const Range& r : t.ranges) {
            QJsonObject ro;
            ro["min"] = r.min;
            ro["max"] = r.max;
            ranges.append(ro);
        }
        obj["ranges"] = ranges;
        arr.append(obj);
    }
    root["tests"] = arr;
}

void TestModule::addTest(const Test& t)
{
    Test copy = t;
    copy.id = generateId();
    m_tests.push_back(copy);
}

bool TestModule::removeById(int id)
{
    for (size_t i = 0; i < m_tests.size(); ++i) {
        if (m_tests[i].id == id) { m_tests.erase(m_tests.begin() + i); return true; }
    }
    return false;
}

bool TestModule::updateTest(const Test& t)
{
    for (Test& ex : m_tests) {
        if (ex.id == t.id) { ex = t; return true; }
    }
    return false;
}

std::optional<Test> TestModule::findByCanId(int canId) const
{
    for (const Test& t : m_tests) if (t.canId == canId) return t;
    return std::nullopt;
}

const std::vector<Test>& TestModule::tests() const { return m_tests; }

int TestModule::generateId() { return m_nextId++; }

bool TestModule::runTestByCanId(int canId, const std::vector<int>& values, QString* optionalReason) const
{
    auto maybe = findByCanId(canId);
    if (!maybe.has_value()) {
        if (optionalReason) *optionalReason = "Test not found for canId";
        return false;
    }
    const Test& t = *maybe;

    if (t.ranges.empty()) {
        if (optionalReason) *optionalReason = "No ranges defined for test";
        return false;
    }

    // один диапазон для всех значений
    if (t.ranges.size() == 1) {
        const Range& r = t.ranges.front();
        for (size_t i = 0; i < values.size(); ++i) {
            int v = values[i];
            if (v < r.min || v > r.max) {
                if (optionalReason) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "Value at index %zu = %d out of range [%d,%d]", i, v, r.min, r.max);
                    *optionalReason = QString::fromUtf8(buf);
                }
                return false;
            }
        }
        return true;
    }

    // проверка попарно
    if (t.ranges.size() == values.size()) {
        for (size_t i = 0; i < values.size(); ++i) {
            int v = values[i];
            const Range& r = t.ranges[i];
            if (v < r.min || v > r.max) {
                if (optionalReason) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "Value at index %zu = %d out of range [%d,%d]", i, v, r.min, r.max);
                    *optionalReason = QString::fromUtf8(buf);
                }
                return false;
            }
        }
        return true;
    }

    if (optionalReason) *optionalReason = "Ranges count doesn't match values count";
    return false;
}
