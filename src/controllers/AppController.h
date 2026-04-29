#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include "../serial/SerialConnection.h"
#include "../commands/CommandStore.h"
#include "../scenarios/ScenarioStore.h"
#include "../scenarios/ScenarioRunner.h"
#include "../decoder/FilterStore.h"
#include "../decoder/CanDecoder.h"

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject* parent = nullptr);

    // ── Файл (команды + сценарии) ─────────────────────────────────────────────
    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath) const;

    // ── Фильтры (отдельный файл) ──────────────────────────────────────────────
    bool loadFilters(const QString& filePath);
    bool saveFilters(const QString& filePath) const;

    // ── Подключение ──────────────────────────────────────────────────────────
    bool connectSerial(const QString& port, int baudRate);
    void disconnectSerial();
    bool isConnected() const;

    // ── Команды ──────────────────────────────────────────────────────────────
    void addCommand(const QString& name, const QString& text);
    bool removeCommand(int id);
    bool updateCommand(const Command& cmd);
    const QList<Command>& commands() const;

    // ── Сценарии ─────────────────────────────────────────────────────────────
    void addScenario(const Scenario& s);
    bool removeScenario(int id);
    bool updateScenario(const Scenario& s);
    const QList<Scenario>& scenarios() const;
    void runScenario(int id);
    void stopScenario();
    bool isScenarioRunning() const;
    void onAdapterReady();

    // ── Фильтры декодера ─────────────────────────────────────────────────────
    FilterStore* filterStore() const;
    void setDecoderEnabled(bool enabled);
    bool isDecoderEnabled() const;

    // ── Отправка ─────────────────────────────────────────────────────────────
    bool sendRaw(const QString& text);
    bool sendByName(const QString& name);
    bool sendById(int id);

signals:
    void dataReceived(const QString& data);
    void commandSent(const QString& text);
    void connectionError(const QString& message);
    void scenarioStepSent(const QString& cmd, int stepNum, int total,
        const QString& scenarioName);
    void scenarioFinished(const QString& scenarioName);
    void scenarioStopped(const QString& scenarioName);
    void decodedValue(const QString& formatted);

private slots:
    bool sendFromScenario(const QString& text);
    void tryDecode(const QString& data);

private:
    std::unique_ptr<SerialConnection> m_connection;
    std::unique_ptr<CommandStore>     m_store;
    std::unique_ptr<ScenarioStore>    m_scenarioStore;
    std::unique_ptr<ScenarioRunner>   m_runner;
    std::unique_ptr<FilterStore>      m_filterStore;
    std::unique_ptr<CanDecoder>       m_decoder;

    QString m_lineBuffer;
};