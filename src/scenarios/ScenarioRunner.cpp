#include "ScenarioRunner.h"

ScenarioRunner::ScenarioRunner(QObject* parent)
    : QObject(parent)
{
    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &ScenarioRunner::onTimeout);
}

bool ScenarioRunner::isRunning() const { return m_running; }

void ScenarioRunner::run(const Scenario& scenario)
{
    if (m_running) stop();

    m_steps = scenario.steps;
    m_scenarioName = scenario.name;
    m_currentStep = 0;
    m_running = true;
    m_waitingReady = false;

    m_timeoutTimer.setInterval(scenario.delayMs);

    // Первую команду отправляем через singleShot(0) — так же как adapterReady,
    // чтобы первый шаг попадал в event loop уже после инициализации
    QTimer::singleShot(0, this, &ScenarioRunner::sendNextStep);
}

void ScenarioRunner::stop()
{
    if (!m_running) return;
    m_timeoutTimer.stop();
    m_running = false;
    m_waitingReady = false;
    emit stopped(m_scenarioName);
}

void ScenarioRunner::sendNextStep()
{
    if (!m_running) return;

    if (m_currentStep >= m_steps.size()) {
        m_running = false;
        m_waitingReady = false;
        emit finished(m_scenarioName);
        return;
    }

    const QString cmd = m_steps.at(m_currentStep);
    const int stepNum = m_currentStep + 1;
    const int total = m_steps.size();
    ++m_currentStep;

    emit sendCommand(cmd);
    emit stepSent(cmd, stepNum, total, m_scenarioName);

    // Ждём '>' от адаптера
    m_waitingReady = true;
    m_timeoutTimer.start();
}

void ScenarioRunner::onAdapterReady()
{
    // Игнорируем если не ждём — защита от лишних '>' в потоке данных
    if (!m_running || !m_waitingReady) return;

    // Атомарно сбрасываем флаг — следующий adapterReady будет проигнорирован
    // пока sendNextStep не выставит его снова
    m_waitingReady = false;
    m_timeoutTimer.stop();

    // Отправляем следующий шаг через singleShot(0) —
    // даём event loop обработать все pending события (вывод в терминал и т.д.)
    // прежде чем следующая команда уйдёт в порт
    QTimer::singleShot(0, this, &ScenarioRunner::sendNextStep);
}

void ScenarioRunner::onTimeout()
{
    if (!m_running) return;
    m_waitingReady = false;
    sendNextStep();
}