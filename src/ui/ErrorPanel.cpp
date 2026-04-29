#include "ErrorPanel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidgetItem>
#include <QLabel>
#include "../style/AppStyle.h"

ErrorPanel::ErrorPanel(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("errorPanel");
    setStyleSheet(R"(
        #errorPanel { background-color: transparent; }
    )");

    QVBoxLayout* main = new QVBoxLayout(this);
    main->setSpacing(6);
    main->setContentsMargins(0, 0, 0, 0);

    // Заголовок с кнопкой очистки и сворачивания
    QHBoxLayout* header = new QHBoxLayout();
    // compact header: title at left, small action buttons at right
    QLabel* title = new QLabel("Ошибки", this);
    title->setStyleSheet("color: #B84040; font-weight: 600; padding-left:6px;");
    header->addWidget(title);
    header->addStretch(1);

    // Refresh button (small)
    m_refreshBtn = new QPushButton("⟳", this);
    m_refreshBtn->setFixedSize(28, 28);
    m_refreshBtn->setStyleSheet("border: 1px solid #D8D2C8; border-radius: 6px; background: #EAE6E0;");
    header->addWidget(m_refreshBtn);

    m_toggleBtn = new QPushButton("∧", this);
    m_toggleBtn->setFixedSize(28, 28);
    m_toggleBtn->setStyleSheet("border: 1px solid #D8D2C8; border-radius: 6px; background: #EAE6E0;");
    header->addWidget(m_toggleBtn);

    m_clearBtn = new QPushButton("Очистить", this);
    AppStyle::applyDanger(m_clearBtn);
    m_clearBtn->setFixedSize(28, 28);
    header->addWidget(m_clearBtn);
    main->addLayout(header);

    m_list = new QListWidget(this);
    m_list->setSelectionMode(QAbstractItemView::NoSelection);
    m_list->setFocusPolicy(Qt::NoFocus);
    m_list->setStyleSheet(R"(
        QListWidget { background-color: #F5F2EE; border: 1px solid #D8D2C8; border-radius: 6px; }
        QListWidget::item { padding: 6px 8px; }
    )");
    main->addWidget(m_list);

    connect(m_clearBtn, &QPushButton::clicked, this, &ErrorPanel::onClearAll);
    connect(m_toggleBtn, &QPushButton::clicked, this, &ErrorPanel::toggleCollapsed);
    connect(m_refreshBtn, &QPushButton::clicked, this, &ErrorPanel::onRefreshClicked);
}

void ErrorPanel::addError(const QString& message)
{
    QListWidgetItem* it = new QListWidgetItem(message, m_list);
    QWidget* w = new QWidget();
    QHBoxLayout* l = new QHBoxLayout(w);
    l->setContentsMargins(6, 2, 6, 2);
    QLabel* lbl = new QLabel(message, w);
    lbl->setStyleSheet("color: #B84040; font-size:12px;");
    QPushButton* closeBtn = new QPushButton("✕", w);
    closeBtn->setFixedSize(20, 20);
    closeBtn->setStyleSheet("border: none; background: transparent; color: #B84040;");
    l->addWidget(lbl);
    l->addStretch(1);
    l->addWidget(closeBtn);
    it->setSizeHint(QSize(w->sizeHint().width(), 28));
    m_list->setItemWidget(it, w);

    connect(closeBtn, &QPushButton::clicked, [this, it]() {
        delete m_list->takeItem(m_list->row(it));
    });

    // Если панель свернута — развернуть при добавлении новой ошибки
    if (m_collapsed) toggleCollapsed();
}

void ErrorPanel::onClearAll()
{
    m_list->clear();
}

void ErrorPanel::toggleCollapsed()
{
    m_collapsed = !m_collapsed;
    m_list->setVisible(!m_collapsed);
    m_clearBtn->setVisible(!m_collapsed);
    m_refreshBtn->setVisible(!m_collapsed);
    m_toggleBtn->setText(m_collapsed ? "∨" : "∧");
}

void ErrorPanel::onRefreshClicked()
{
    // For now just trim the list to most recent 50 entries to keep panel compact
    const int maxItems = 50;
    while (m_list->count() > maxItems) {
        delete m_list->takeItem(0);
    }
}

void ErrorPanel::refresh()
{
    onRefreshClicked();
}
