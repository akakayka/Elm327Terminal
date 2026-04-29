#pragma once

#include <QWidget>
#include <QListWidget>
#include <QPushButton>

// Простая панель ошибок, отображающая список сообщений с возможностью закрытия.
class ErrorPanel : public QWidget {
    Q_OBJECT
public:
    explicit ErrorPanel(QWidget* parent = nullptr);

public slots:
    void addError(const QString& message);
    void toggleCollapsed();
    void refresh();

private slots:
    void onClearAll();
    void onRefreshClicked();

private:
    QListWidget* m_list;
    QPushButton* m_clearBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_toggleBtn;
    bool m_collapsed = false;
};
