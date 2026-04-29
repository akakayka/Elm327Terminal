#include "AppController.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

AppController::AppController(QObject* parent)
    : QObject(parent)
    , m_connection(std::make_unique<SerialConnection>())
    , m_store(std::make_unique<CommandStore>())
    , m_scenarioStore(std::make_unique<ScenarioStore>())
    , m_runner(std::make_unique<ScenarioRunner>())
    , m_filterStore(std::make_unique<FilterStore>())
    , m_decoder(std::make_unique<CanDecoder>(m_filterStore.get()))
{
    connect(m_connection.get(), &SerialConnection::dataReceived,
        this, &AppController::dataReceived);
    connect(m_connection.get(), &SerialConnection::errorOccurred,
        this, &AppController::connectionError);
    connect(m_connection.get(), &SerialConnection::dataReceived,
        this, &AppController::tryDecode);

    connect(m_runner.get(), &ScenarioRunner::sendCommand,
        this, &AppController::sendFromScenario);
    connect(m_runner.get(), &ScenarioRunner::stepSent,
        this, &AppController::scenarioStepSent);
    connect(m_runner.get(), &ScenarioRunner::finished,
        this, &AppController::scenarioFinished);
    connect(m_runner.get(), &ScenarioRunner::stopped,
        this, &AppController::scenarioStopped);
}

// ── Файл ──────────────────────────────────────────────────────────────────────

bool AppController::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return false;
    QJsonObject root = doc.object();
    m_store->loadFromJson(root);
    m_scenarioStore->loadFromJson(root);
    return true;
}

bool AppController::saveToFile(const QString& filePath) const
{
    QJsonObject root;
    m_store->saveToJson(root);
    m_scenarioStore->saveToJson(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool AppController::loadFilters(const QString& filePath)
{
    return m_filterStore->loadFromFile(filePath);
}

bool AppController::saveFilters(const QString& filePath) const
{
    return m_filterStore->saveToFile(filePath);
}

// ── Подключение ───────────────────────────────────────────────────────────────

bool AppController::connectSerial(const QString& port, int baudRate)
{
    return m_connection->connectPort(port, baudRate);
}
void AppController::disconnectSerial() { m_connection->disconnectPort(); }
bool AppController::isConnected() const { return m_connection->isOpen(); }

// ── Команды ───────────────────────────────────────────────────────────────────

void AppController::addCommand(const QString& name, const QString& text)
{
    Command cmd; cmd.id = 0; cmd.name = name; cmd.text = text;
    m_store->addCommand(cmd);
}
bool AppController::removeCommand(int id) { return m_store->removeById(id); }
bool AppController::updateCommand(const Command& c) { return m_store->updateCommand(c); }
const QList<Command>& AppController::commands() const { return m_store->commands(); }

// ── Сценарии ──────────────────────────────────────────────────────────────────

void AppController::addScenario(const Scenario& s) { m_scenarioStore->addScenario(s); }
bool AppController::removeScenario(int id) { return m_scenarioStore->removeById(id); }
bool AppController::updateScenario(const Scenario& s) { return m_scenarioStore->updateScenario(s); }
const QList<Scenario>& AppController::scenarios() const { return m_scenarioStore->scenarios(); }

void AppController::runScenario(int id)
{
    auto s = m_scenarioStore->findById(id);
    if (s.has_value()) m_runner->run(*s);
}
void AppController::stopScenario() { m_runner->stop(); }
bool AppController::isScenarioRunning() const { return m_runner->isRunning(); }
void AppController::onAdapterReady() { m_runner->onAdapterReady(); }

// ── Декодер ───────────────────────────────────────────────────────────────────

FilterStore* AppController::filterStore() const { return m_filterStore.get(); }
void AppController::setDecoderEnabled(bool e) { m_decoder->setEnabled(e); }
bool AppController::isDecoderEnabled() const { return m_decoder->isEnabled(); }

// ── Отправка ──────────────────────────────────────────────────────────────────

bool AppController::sendRaw(const QString& text)
{
    if (!m_connection->send(text)) return false;
    emit commandSent(text);
    return true;
}

bool AppController::sendByName(const QString& name)
{
    auto cmd = m_store->findByName(name);
    if (!cmd.has_value()) return false;
    return sendRaw(cmd->text);
}

bool AppController::sendById(int id)
{
    auto cmd = m_store->findById(id);
    if (!cmd.has_value()) return false;
    return sendRaw(cmd->text);
}

bool AppController::sendFromScenario(const QString& text)
{
    return m_connection->send(text);
}

// ── Декодирование ─────────────────────────────────────────────────────────────

void AppController::tryDecode(const QString& data)
{
    if (!m_decoder->isEnabled()) return;

    m_lineBuffer += data;

    while (true) {
        int pos = -1;
        for (int i = 0; i < m_lineBuffer.size(); ++i) {
            QChar c = m_lineBuffer[i];
            if (c == '\r' || c == '\n' || c == '>') { pos = i; break; }
        }
        if (pos < 0) break;

        const QString line = m_lineBuffer.left(pos).trimmed();
        m_lineBuffer = m_lineBuffer.mid(pos + 1);
        if (line.isEmpty()) continue;

        const QList<DecodeResult> results = m_decoder->decode(line);
        for (const DecodeResult& r : results)
            emit decodedValue(r.formatted);
    }
}