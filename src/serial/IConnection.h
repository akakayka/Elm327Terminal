#pragma once

#include <QString>

class IConnection
{
public:
    virtual ~IConnection() = default;

    virtual bool connect(const QString& portName, int baudRate) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual bool sendCommand(const QString& command) = 0;
    virtual QString readResponse() = 0;
};