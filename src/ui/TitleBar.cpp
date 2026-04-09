#include "TitleBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QWindow>
#include <QScreen>
#include <QApplication>

TitleBar::TitleBar(QWidget* parent)
    : QWidget(parent)
{
    setFixedHeight(42);
    setupUi();
}

void TitleBar::setupUi()
{
    setStyleSheet(R"(
        TitleBar {
            background-color: #EAE6E0;
            border-bottom: 1px solid #D8D2C8;
            border-top-left-radius: 12px;
            border-top-right-radius: 12px;
        }
    )");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(14, 0, 8, 0);
    layout->setSpacing(5);

    // ── Логотип ────────────────────────────────────────────────────────────────
    QLabel* logo = new QLabel("ELM", this);
    logo->setFixedSize(36, 22);
    logo->setAlignment(Qt::AlignCenter);
    logo->setStyleSheet(
        "background-color: #5B6EC7;"
        "color: #FFFFFF;"
        "font-weight: 700;"
        "font-size: 11px;"
        "letter-spacing: 1px;"
        "border-radius: 4px;"
        "text-align: center;"
    );

    QLabel* num = new QLabel("327", this);
    num->setStyleSheet(
        "color: #2C2A27;"
        "font-size: 16px;"
        "font-weight: 700;"
        "letter-spacing: -0.5px;"
    );

    QLabel* word = new QLabel("Terminal", this);
    word->setStyleSheet(
        "color: #2C2A27;"
        "font-size: 16px;"
        "font-weight: 400;"
        "letter-spacing: -0.5px;"
    );

    QLabel* ver = new QLabel("v1.0", this);
    ver->setStyleSheet(
        "color: #B0A898;"
        "font-size: 11px;"
        "font-weight: 400;"
        "margin-left: 2px;"
        "letter-spacing: -0.5px;"
    );

    layout->addWidget(logo);
    layout->addSpacing(2);
    layout->addWidget(num);
    layout->addWidget(word);
    layout->addWidget(ver);
    layout->addStretch();

    // ── Кнопки управления окном ───────────────────────────────────────────────
    const QString btnBase = R"(
        QPushButton {
            border: none;
            border-radius: 6px;
            font-size: 14px;
            font-weight: 500;
            min-width: 32px;
            min-height: 28px;
            max-width: 32px;
            max-height: 28px;
            background-color: transparent;
            color: #8A8278;
        }
        QPushButton:hover {
            background-color: #D8D2C8;
            color: #2C2A27;
        }
    )";

    const QString closeBtnStyle = R"(
        QPushButton {
            border: none;
            border-radius: 6px;
            font-size: 16px;
            font-weight: 500;
            min-width: 32px;
            min-height: 28px;
            max-width: 32px;
            max-height: 28px;
            background-color: transparent;
            color: #8A8278;
        }
        QPushButton:hover {
            background-color: #E81123;
            color: #FFFFFF;
        }
    )";

    m_minimizeBtn = new QPushButton("−", this);
    m_maximizeBtn = new QPushButton("□", this);
    m_closeBtn = new QPushButton("✕", this);

    m_minimizeBtn->setStyleSheet(btnBase);
    m_maximizeBtn->setStyleSheet(btnBase);
    m_closeBtn->setStyleSheet(closeBtnStyle);

    m_minimizeBtn->setToolTip("Свернуть");
    m_maximizeBtn->setToolTip("Развернуть");
    m_closeBtn->setToolTip("Закрыть");

    layout->addWidget(m_minimizeBtn);
    layout->addWidget(m_maximizeBtn);
    layout->addWidget(m_closeBtn);

    connect(m_minimizeBtn, &QPushButton::clicked, this, &TitleBar::onMinimize);
    connect(m_maximizeBtn, &QPushButton::clicked, this, &TitleBar::onMaximize);
    connect(m_closeBtn, &QPushButton::clicked, this, &TitleBar::onClose);
}

void TitleBar::updateMaximizeButton(bool isMaximized)
{
    m_maximizeBtn->setText(isMaximized ? "❐" : "□");
    m_maximizeBtn->setToolTip(isMaximized ? "Восстановить" : "Развернуть");
}

bool TitleBar::canDragWindow() const
{
    // Проверяем, что окно существует и не максимизировано
    return window() && !window()->isMaximized();
}

// ── Кнопки ────────────────────────────────────────────────────────────────────

void TitleBar::onMinimize()
{
    if (window()) {
        window()->showMinimized();
    }
}

void TitleBar::onMaximize()
{
    if (!window()) return;

    if (window()->isMaximized()) {
        window()->showNormal();
        updateMaximizeButton(false);
    }
    else {
        window()->showMaximized();
        updateMaximizeButton(true);
    }

    // Испускаем сигнал для дополнительной логики в MainWindow
    emit maximizeRequested();
}

void TitleBar::onClose()
{
    if (window()) {
        window()->close();
    }
}

// ── Перетаскивание окна ───────────────────────────────────────────────────────

void TitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && canDragWindow()) {
        m_dragging = true;
        m_dragStart = event->globalPosition().toPoint();
        m_windowStart = window()->pos();
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_dragging || !canDragWindow()) return;

    const QPoint delta = event->globalPosition().toPoint() - m_dragStart;
    window()->move(m_windowStart + delta);
    event->accept();
}

void TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
    QWidget::mouseReleaseEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        onMaximize();
        event->accept();
    }
    else {
        QWidget::mouseDoubleClickEvent(event);
    }
}