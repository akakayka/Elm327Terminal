#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QJsonObject>
#include <optional>
#include "Command.h"

class CommandStore : public QObject {
    Q_OBJECT

public:
    explicit CommandStore(QObject* parent = nullptr);

    // Теперь работает с QJsonObject — чтобы один файл содержал и команды и сценарии
    bool loadFromJson(const QJsonObject& root);
    void saveToJson(QJsonObject& root) const;

    void addCommand(const Command& cmd);
    bool removeById(int id);
    bool updateCommand(const Command& cmd);

    std::optional<Command> findById(int id) const;
    std::optional<Command> findByName(const QString& name) const;
    const QList<Command>& commands() const;

private:
    QList<Command> m_commands;
    int            m_nextId = 1;
    int            generateId();
};