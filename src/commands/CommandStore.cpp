#include "CommandStore.h"
#include <QJsonArray>
#include <QJsonObject>

CommandStore::CommandStore(QObject* parent)
    : QObject(parent)
{
}

bool CommandStore::loadFromJson(const QJsonObject& root)
{
    m_commands.clear();
    m_nextId = 1;

    const QJsonArray arr = root.value("commands").toArray();
    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        Command cmd;
        cmd.id = obj.value("id").toInt();
        cmd.name = obj.value("name").toString();
        cmd.text = obj.value("text").toString();

        if (cmd.isValid()) {
            m_commands.append(cmd);
            if (cmd.id >= m_nextId)
                m_nextId = cmd.id + 1;
        }
    }
    return true;
}

void CommandStore::saveToJson(QJsonObject& root) const
{
    QJsonArray arr;
    for (const Command& cmd : m_commands) {
        QJsonObject obj;
        obj["id"] = cmd.id;
        obj["name"] = cmd.name;
        obj["text"] = cmd.text;
        arr.append(obj);
    }
    root["commands"] = arr;
}

void CommandStore::addCommand(const Command& cmd)
{
    Command c = cmd;
    c.id = generateId();
    m_commands.append(c);
}

bool CommandStore::removeById(int id)
{
    for (int i = 0; i < m_commands.size(); ++i) {
        if (m_commands[i].id == id) {
            m_commands.removeAt(i);
            return true;
        }
    }
    return false;
}

bool CommandStore::updateCommand(const Command& cmd)
{
    for (Command& c : m_commands) {
        if (c.id == cmd.id) { c = cmd; return true; }
    }
    return false;
}

std::optional<Command> CommandStore::findById(int id) const
{
    for (const Command& c : m_commands)
        if (c.id == id) return c;
    return std::nullopt;
}

std::optional<Command> CommandStore::findByName(const QString& name) const
{
    for (const Command& c : m_commands)
        if (c.name.compare(name, Qt::CaseInsensitive) == 0) return c;
    return std::nullopt;
}

const QList<Command>& CommandStore::commands() const { return m_commands; }

int CommandStore::generateId() { return m_nextId++; }