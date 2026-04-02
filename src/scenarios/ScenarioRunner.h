#pragma once

#include <QObject>
#include <QTimer>
#include <QStringList>
#include "Scenario.h"

// ScenarioRunner отправляет следующий шаг только когда:
// - получен сигнал adapterReady (адаптер прислал '>'), ИЛИ
// - истёк таймаут (защита от зависания если '>' не пришёл)
class ScenarioRunner : public QObject {
    Q_OBJECT

public:
    explicit ScenarioRunner(QObject* parent = nullptr);

    bool isRunning() const;

public slots:
    void run(const Scenario& scenario);
    void stop();

    // Вызывается из TerminalPanel когда адаптер прислал '>'
    void onAdapterReady();

signals:
    void stepSent(const QString& command, int stepNum, int totalSteps,
        const QString& scenarioName);
    void finished(const QString& scenarioName);
    void stopped(const QString& scenarioName);
    void sendCommand(const QString& command);

private slots:
    void sendNextStep();
    void onTimeout();

private:
    QTimer      m_timeoutTimer;  // максимальное ожидание ответа
    QStringList m_steps;
    QString     m_scenarioName;
    int         m_currentStep = 0;
    bool        m_running = false;
    bool        m_waitingReady = false; // ждём '>' от адаптера
};