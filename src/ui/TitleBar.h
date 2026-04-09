#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QPoint>

class TitleBar : public QWidget {
    Q_OBJECT

public:
    explicit TitleBar(QWidget* parent = nullptr);

    void updateMaximizeButton(bool isMaximized);

    // Добавлен сигнал для уведомления MainWindow
signals:
    void maximizeRequested();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void onMinimize();
    void onMaximize();
    void onClose();

private:
    void setupUi();

    // Функция для проверки, можно ли перетаскивать окно
    bool canDragWindow() const;

    QPushButton* m_minimizeBtn = nullptr;
    QPushButton* m_maximizeBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;

    bool   m_dragging = false;
    QPoint m_dragStart;
    QPoint m_windowStart;
};