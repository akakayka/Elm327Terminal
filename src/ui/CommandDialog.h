#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>

#include "../commands/Command.h"

// Диалог для добавления и редактирования команды.
// Используется как для "Добавить", так и для "Изменить" —
// при редактировании просто передаём существующую команду.
class CommandDialog : public QDialog {
    Q_OBJECT

public:
    // Создать пустой диалог (режим добавления)
    explicit CommandDialog(QWidget* parent = nullptr);

    // Создать диалог с заполненными полями (режим редактирования)
    explicit CommandDialog(const Command& cmd, QWidget* parent = nullptr);

    // Получить результат после принятия диалога
    QString name() const;
    QString text() const;

private:
    void setupUi(const QString& title);
    void validate(); // включить/выключить OK в зависимости от заполненности

    QLineEdit* m_nameEdit;
    QLineEdit* m_textEdit;
    QDialogButtonBox* m_buttons;
};