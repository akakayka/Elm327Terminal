#pragma once
#include <QString>
#include <QPushButton>

namespace AppStyle {

    // ── Постельная палитра ────────────────────────────────────────────────────────
    constexpr auto COLOR_BG = "#F0EDE8";  // тёплый кремовый фон
    constexpr auto COLOR_BG_PANEL = "#EAE6E0";  // чуть темнее для панелей
    constexpr auto COLOR_BG_INPUT = "#F5F2EE";  // инпуты
    constexpr auto COLOR_BORDER = "#D8D2C8";  // тёплые границы
    constexpr auto COLOR_BORDER_FOCUS = "#6B7FCC";  // фокус — приглушённый индиго
    constexpr auto COLOR_ACCENT = "#5B6EC7";  // акцент — приглушённый индиго
    constexpr auto COLOR_ACCENT_HOVER = "#4A5DB5";  // акцент hover
    constexpr auto COLOR_ACCENT_TEXT = "#FFFFFF";
    constexpr auto COLOR_TEXT = "#2C2A27";  // тёплый почти-чёрный
    constexpr auto COLOR_TEXT_MUTED = "#8A8278";  // приглушённый тёплый серый
    constexpr auto COLOR_SUCCESS = "#3D7A52";  // приглушённый зелёный
    constexpr auto COLOR_ERROR = "#B84040";  // приглушённый красный

    // Терминал
    constexpr auto TERM_BG = "#EEEAE4";
    constexpr auto TERM_SENT = "#3D7A52";
    constexpr auto TERM_RECV = "#2A5490";
    constexpr auto TERM_SYSTEM = "#A09890";
    constexpr auto TERM_DECODED = "#8B6914";
    constexpr auto TERM_SCENARIO = "#6B4FA0";

    inline QString globalStyleSheet() {
        return R"STYLE(
QWidget {
    font-family: "Segoe UI", "SF Pro Text", sans-serif;
    font-size: 13px;
    color: #2C2A27;
    background-color: #F0EDE8;
}
QMainWindow {
    background-color: #EAE6E0;
}
QMainWindow::centralWidget {
    background-color: #EAE6E0;
    border: 1px solid #C8C2B8;
    border-radius: 0px;
}

/* GroupBox — без заголовка, просто контейнер */
QGroupBox {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 8px;
    margin-top: 0px;
    padding: 10px;
    font-size: 0px;
    color: transparent;
}
QGroupBox::title {
    font-size: 0px;
    color: transparent;
    height: 0px;
    padding: 0px;
    margin: 0px;
}

QPushButton {
    background-color: #EAE6E0;
    color: #2C2A27;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    padding: 6px 14px;
    font-weight: 500;
    font-size: 13px;
    min-height: 30px;
    text-align: center;
    vertical-align: middle;
}
QPushButton:hover  { background-color: #E0DBD4; border-color: #C4BDB3; }
QPushButton:pressed { background-color: #D8D2CB; }
QPushButton:disabled { color: #C0B8AE; border-color: #E0DBD4; background-color: #EAE6E0; }

QPushButton[accent="true"] {
    background-color: #5B6EC7;
    color: #FFFFFF;
    border: none;
    font-weight: 600;
}
QPushButton[accent="true"]:hover   { background-color: #4A5DB5; }
QPushButton[accent="true"]:pressed { background-color: #3D4E9A; }
QPushButton[accent="true"]:disabled { background-color: #A8B0D8; color: #FFFFFF; }

QPushButton[danger="true"] {
    background-color: #F5EEEE;
    color: #B84040;
    border: 1px solid #E8CECE;
}
QPushButton[danger="true"]:hover { background-color: #EDE4E4; border-color: #DDB8B8; }
QPushButton[danger="true"]:disabled { color: #C8A0A0; border-color: #E8DADA; }

QComboBox {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    padding: 5px 10px;
    min-height: 30px;
    color: #2C2A27;
}
QComboBox:hover { border-color: #C4BDB3; }
QComboBox:focus { border-color: #5B6EC7; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    selection-background-color: #DDE0F0;
    selection-color: #3D4E9A;
    padding: 4px;
}

QLineEdit {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    padding: 5px 10px;
    min-height: 30px;
    color: #2C2A27;
}
QLineEdit:hover { border-color: #C4BDB3; }
QLineEdit:focus { border-color: #5B6EC7; }

QPlainTextEdit, QTextEdit {
    background-color: #EEEAE4;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    padding: 8px;
    color: #2C2A27;
}
QPlainTextEdit:focus, QTextEdit:focus { border-color: #5B6EC7; }

QLabel { background-color: transparent; color: #2C2A27; }

QCheckBox { spacing: 8px; color: #2C2A27; }
QCheckBox::indicator {
    width: 16px; height: 16px;
    border: 1.5px solid #C4BDB3;
    border-radius: 4px;
    background-color: #F5F2EE;
}
QCheckBox::indicator:hover   { border-color: #5B6EC7; }
QCheckBox::indicator:checked { background-color: #5B6EC7; border-color: #5B6EC7; image: url(:/style/icons/icons8-done.svg); }

QListWidget {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    padding: 4px;
    outline: none;
}
QListWidget::item { border-radius: 4px; padding: 5px 8px; }
QListWidget::item:hover    { background-color: #E8E4F4; }
QListWidget::item:selected { background-color: #DDE0F0; color: #3D4E9A; font-weight: 500; }
QListWidget::item:alternate { background-color: #EEE9E3; }

QTableWidget {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    gridline-color: #E8E3DC;
    outline: none;
}
QTableWidget::item          { padding: 6px 10px; border: none; }
QTableWidget::item:selected { background-color: #DDE0F0; color: #3D4E9A; }
QHeaderView::section {
    background-color: #EAE6E0;
    border: none;
    border-bottom: 1px solid #D8D2C8;
    padding: 6px 10px;
    font-weight: 600;
    font-size: 11px;
    color: #8A8278;
    letter-spacing: 0.3px;
    text-transform: uppercase;
}

QTabWidget::pane {
    border: 1px solid #D8D2C8;
    border-radius: 8px;
    background-color: #F5F2EE;
    top: -1px;
}
QTabBar { background: transparent; }
QTabBar::tab {
    background-color: transparent;
    border: none;
    border-bottom: 2px solid transparent;
    padding: 8px 20px;
    color: #8A8278;
    font-weight: 500;
    margin-right: 2px;
}
QTabBar::tab:hover    { color: #5B6EC7; background-color: #E8E4F0; border-radius: 6px 6px 0 0; }
QTabBar::tab:selected { color: #5B6EC7; border-bottom: 2px solid #5B6EC7; font-weight: 600; }

QSplitter::handle            { background-color: #D8D2C8; }
QSplitter::handle:vertical   { height: 1px; }
QSplitter::handle:horizontal { width: 1px; }
QSplitter::handle:hover      { background-color: #5B6EC7; }

QScrollBar:vertical       { background: transparent; width: 8px; margin: 0; }
QScrollBar::handle:vertical { background: #C8C2B8; border-radius: 4px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: #B0AAA0; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal     { background: transparent; height: 8px; }
QScrollBar::handle:horizontal { background: #C8C2B8; border-radius: 4px; min-width: 30px; }
QScrollBar::handle:horizontal:hover { background: #B0AAA0; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }

QStatusBar {
    background-color: #E8E4DE;
    border-top: 1px solid #D8D2C8;
    color: #8A8278;
    font-size: 12px;
    padding: 4px 40px !important;
    min-height: 24px;
    border-bottom-left-radius: 12px;
    border-bottom-right-radius: 12px;
}
QStatusBar::item { border: none; margin-left: 20px; margin-right: 20px; }

QMenuBar {
    background-color: #EAE6E0;
    border-bottom: 1px solid #D8D2C8;
    padding: 2px 4px;
    color: #2C2A27;
}
QMenuBar::item { background: transparent; padding: 4px 10px; border-radius: 4px; }
QMenuBar::item:selected { background-color: #E0DBF0; color: #3D4E9A; }
QMenu {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 8px;
    padding: 4px;
}
QMenu::item { padding: 6px 28px 6px 12px; border-radius: 4px; color: #2C2A27; }
QMenu::item:selected { background-color: #DDE0F0; color: #3D4E9A; }
QMenu::separator { height: 1px; background: #D8D2C8; margin: 4px 8px; }

QSpinBox, QDoubleSpinBox {
    background-color: #F5F2EE;
    border: 1px solid #D8D2C8;
    border-radius: 6px;
    padding: 5px 8px;
    min-height: 30px;
    color: #2C2A27;
}
QSpinBox:focus, QDoubleSpinBox:focus { border-color: #5B6EC7; }
QSpinBox::up-button, QSpinBox::down-button,
QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { border: none; background: transparent; width: 16px; }

QDialog { background-color: #F0EDE8; }
QDialogButtonBox QPushButton { min-width: 80px; }

QToolTip {
    background-color: #2C2A27;
    color: #F5F2EE;
    border: none;
    border-radius: 4px;
    padding: 4px 8px;
    font-size: 12px;
}
)STYLE";
    }

    // ── Вспомогательные функции для стилизации кнопок ────────────────────────────
    // Используем прямой setStyleSheet вместо property() — надёжнее при
    // Qt::FramelessWindowHint и динамическом создании виджетов

    inline void applyAccent(QPushButton* btn)
    {
        btn->setStyleSheet(R"(
        QPushButton {
            background-color: #5B6EC7;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: 600;
            font-size: 13px;
            min-height: 32px;
        }
        QPushButton:hover    { background-color: #4A5DB5; }
        QPushButton:pressed  { background-color: #3D4E9A; }
        QPushButton:disabled { background-color: #A8B0D8; color: #FFFFFF; }
    )");
    }

    inline void applyDanger(QPushButton* btn)
    {
        btn->setStyleSheet(R"(
        QPushButton {
            background-color: #F5EEEE;
            color: #B84040;
            border: 1px solid #E8CECE;
            border-radius: 6px;
            padding: 6px 16px;
            font-weight: 500;
            font-size: 13px;
            min-height: 32px;
        }
        QPushButton:hover    { background-color: #EDE4E4; border-color: #DDB8B8; }
        QPushButton:pressed  { background-color: #E0D8D8; }
        QPushButton:disabled { color: #C8A0A0; border-color: #E8DADA; background-color: #F5EEEE; }
    )");
    }

} // namespace AppStyle