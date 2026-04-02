#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QDialogButtonBox>

#include "../scenarios/Scenario.h"
#include "../commands/Command.h"

// Диалог создания/редактирования сценария.
// Слева — выпадающий список всех команд для добавления шага.
// Справа — список шагов сценария с кнопками управления порядком.
class ScenarioDialog : public QDialog {
    Q_OBJECT

public:
    explicit ScenarioDialog(const QList<Command>& commands,
        QWidget* parent = nullptr);
    explicit ScenarioDialog(const Scenario& scenario,
        const QList<Command>& commands,
        QWidget* parent = nullptr);

    Scenario result() const;

private slots:
    void onAddStep();
    void onRemoveStep();
    void onMoveUp();
    void onMoveDown();
    void validate();

private:
    void setupUi();
    void populateSteps(const QStringList& steps);

    QList<Command>    m_commands;
    Scenario          m_original; // для режима редактирования

    QLineEdit* m_nameEdit;
    QTextEdit* m_descEdit;
    QSpinBox* m_delaySpin;

    // Левая часть — доступные команды
    QListWidget* m_commandList;

    // Правая часть — шаги сценария
    QListWidget* m_stepList;

    QDialogButtonBox* m_buttons;
};