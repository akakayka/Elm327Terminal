#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QCheckBox>

class TerminalPanel : public QWidget {
    Q_OBJECT

public:
    explicit TerminalPanel(QWidget* parent = nullptr);

public slots:
    void onCommandSent(const QString& text);
    void onDataReceived(const QString& data);
    void onConnectionError(const QString& message);
    void onSystemMessage(const QString& message);
    void onScenarioStep(const QString& cmd, int stepNum, int total,
        const QString& scenarioName);
    void onScenarioFinished(const QString& scenarioName);
    void onScenarioStopped(const QString& scenarioName);
    // Декодированное значение — выводим отдельным цветом
    void onDecodedValue(const QString& formatted);

signals:
    // Эмитируется когда адаптер прислал '>' — готов к следующей команде
    void adapterReady();

private slots:
    void onClearClicked();
    void onSaveClicked();

private:
    void appendSent(const QString& text);
    void appendReceived(const QString& text);
    void appendSystem(const QString& text);
    void scrollToBottom();

    QPlainTextEdit* m_output;
    QPushButton* m_clearBtn;
    QPushButton* m_saveBtn;
    QCheckBox* m_autoscrollCheck;

    QString m_receiveBuffer;
};