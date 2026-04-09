#include "CommandDialog.h"
#include "../style/AppStyle.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

CommandDialog::CommandDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi("Добавить команду");
}

CommandDialog::CommandDialog(const Command& cmd, QWidget* parent)
    : QDialog(parent)
{
    setupUi("Изменить команду");
    m_nameEdit->setText(cmd.name);
    m_textEdit->setText(cmd.text);
}

void CommandDialog::setupUi(const QString& title)
{
    setWindowTitle(title);
    setMinimumWidth(340);
    setModal(true);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Например: Сброс адаптера");

    m_textEdit = new QLineEdit(this);
    m_textEdit->setPlaceholderText("Например: ATZ");

    m_buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    QFormLayout* form = new QFormLayout;
    form->addRow("Описание:", m_nameEdit);
    form->addRow("Команда:", m_textEdit);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(m_buttons);

    // OK выключен пока поля не заполнены
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(false);
    AppStyle::applyAccent(m_buttons->button(QDialogButtonBox::Ok));

    connect(m_nameEdit, &QLineEdit::textChanged, this, &CommandDialog::validate);
    connect(m_textEdit, &QLineEdit::textChanged, this, &CommandDialog::validate);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CommandDialog::validate()
{
    const bool ok = !m_nameEdit->text().trimmed().isEmpty()
        && !m_textEdit->text().trimmed().isEmpty();
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(ok);
}

QString CommandDialog::name() const { return m_nameEdit->text().trimmed(); }
QString CommandDialog::text() const { return m_textEdit->text().trimmed(); }