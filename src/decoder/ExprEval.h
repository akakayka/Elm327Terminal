#pragma once

#include <QString>
#include <QMap>
#include <QVector>
#include <cmath>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// ExprEval — лёгкий рекурсивный вычислитель арифметических выражений.
//
// Поддерживает:
//   • +  -  *  /  скобки
//   • Унарный минус
//   • Переменные: b0..b7 (байты CAN данных), произвольные через setVar()
//   • Встроенные функции:
//       ieee754(b0,b1,b2,b3)  — big-endian IEEE 754 float из 4 байт
//       hex2dec(b0,b1)        — (b0<<8)|b1 как uint16
//       hex2dec4(b0,b1,b2,b3) — (b0<<24)|... как uint32
//       abs(x)  sqrt(x)  round(x)  floor(x)  ceil(x)
//       min(a,b)  max(a,b)
//
// Пример:
//   ExprEval e;
//   e.setVar("b0", 0x41); e.setVar("b1", 0xA0); ...
//   double result = e.eval("ieee754(b0,b1,b2,b3)");  // → 20.0
// ─────────────────────────────────────────────────────────────────────────────

class ExprEval {
public:
    ExprEval() = default;

    void setVar(const QString& name, double value) {
        m_vars[name] = value;
    }

    // Вычислить выражение. При ошибке возвращает NaN и заполняет error.
    double eval(const QString& expr, QString* error = nullptr) {
        m_src = expr.trimmed();
        m_pos = 0;
        m_error = "";

        double result = parseExpr();

        // Должны дойти до конца строки
        skipWs();
        if (m_pos < m_src.size() && m_error.isEmpty())
            m_error = QString("Unexpected token at pos %1").arg(m_pos);

        if (!m_error.isEmpty()) {
            if (error) *error = m_error;
            return std::numeric_limits<double>::quiet_NaN();
        }
        return result;
    }

    QString lastError() const { return m_error; }

private:
    QString            m_src;
    int                m_pos = 0;
    QString            m_error;
    QMap<QString, double> m_vars;

    // ── Парсер (рекурсивный спуск) ────────────────────────────────────────────

    void skipWs() {
        while (m_pos < m_src.size() && m_src[m_pos].isSpace())
            ++m_pos;
    }

    QChar peek() {
        skipWs();
        return m_pos < m_src.size() ? m_src[m_pos] : QChar('\0');
    }

    QChar consume() {
        skipWs();
        return m_pos < m_src.size() ? m_src[m_pos++] : QChar('\0');
    }

    bool match(QChar c) {
        skipWs();
        if (m_pos < m_src.size() && m_src[m_pos] == c) {
            ++m_pos; return true;
        }
        return false;
    }

    // expr = term (('+' | '-') term)*
    double parseExpr() {
        double left = parseTerm();
        for (;;) {
            if (match('+'))      left += parseTerm();
            else if (match('-')) left -= parseTerm();
            else break;
        }
        return left;
    }

    // term = unary (('*' | '/') unary)*
    double parseTerm() {
        double left = parseUnary();
        for (;;) {
            if (match('*'))      left *= parseUnary();
            else if (match('/')) {
                double r = parseUnary();
                if (r == 0.0) { m_error = "Division by zero"; return 0.0; }
                left /= r;
            }
            else break;
        }
        return left;
    }

    // unary = '-' unary | power
    double parseUnary() {
        if (match('-')) return -parseUnary();
        if (match('+')) return  parseUnary();
        return parsePrimary();
    }

    // primary = number | variable | function '(' args ')' | '(' expr ')'
    double parsePrimary() {
        skipWs();
        if (m_error.size()) return 0.0;

        // Число
        if (m_pos < m_src.size() && (m_src[m_pos].isDigit() || m_src[m_pos] == '.'))
            return parseNumber();

        // Скобки
        if (match('(')) {
            double v = parseExpr();
            if (!match(')')) m_error = "Expected ')'";
            return v;
        }

        // Идентификатор (переменная или функция)
        if (m_pos < m_src.size() && (m_src[m_pos].isLetter() || m_src[m_pos] == '_'))
            return parseIdentifier();

        m_error = QString("Unexpected char '%1' at pos %2")
            .arg(m_pos < m_src.size() ? m_src[m_pos] : QChar('?'))
            .arg(m_pos);
        return 0.0;
    }

    double parseNumber() {
        int start = m_pos;
        // hex literal 0x...
        if (m_pos + 1 < m_src.size() &&
            m_src[m_pos] == '0' &&
            (m_src[m_pos + 1] == 'x' || m_src[m_pos + 1] == 'X'))
        {
            m_pos += 2;
            while (m_pos < m_src.size() && (m_src[m_pos].isLetter() || m_src[m_pos].isDigit())) ++m_pos;
            bool ok;
            double v = m_src.mid(start, m_pos - start).toULongLong(&ok, 16);
            if (!ok) m_error = "Bad hex literal";
            return v;
        }
        while (m_pos < m_src.size() && (m_src[m_pos].isDigit() || m_src[m_pos] == '.')) ++m_pos;
        if (m_pos < m_src.size() && (m_src[m_pos] == 'e' || m_src[m_pos] == 'E')) {
            ++m_pos;
            if (m_pos < m_src.size() && (m_src[m_pos] == '+' || m_src[m_pos] == '-')) ++m_pos;
            while (m_pos < m_src.size() && m_src[m_pos].isDigit()) ++m_pos;
        }
        bool ok;
        double v = m_src.mid(start, m_pos - start).toDouble(&ok);
        if (!ok) m_error = "Bad number";
        return v;
    }

    double parseIdentifier() {
        int start = m_pos;
        while (m_pos < m_src.size() &&
            (m_src[m_pos].isLetter() || m_src[m_pos].isDigit() || m_src[m_pos] == '_')) ++m_pos;
        QString name = m_src.mid(start, m_pos - start);

        skipWs();
        // Функция
        if (m_pos < m_src.size() && m_src[m_pos] == '(') {
            ++m_pos;
            QVector<double> args;
            skipWs();
            if (m_pos < m_src.size() && m_src[m_pos] != ')') {
                args.append(parseExpr());
                while (match(',')) args.append(parseExpr());
            }
            if (!match(')')) m_error = "Expected ')' after function args";
            return callFunction(name, args);
        }

        // Переменная
        if (m_vars.contains(name)) return m_vars[name];
        m_error = QString("Unknown variable '%1'").arg(name);
        return 0.0;
    }

    double callFunction(const QString& name, const QVector<double>& a) {
        auto arg = [&](int i) -> double {
            if (i < a.size()) return a[i];
            m_error = QString("Not enough args for %1").arg(name);
            return 0.0;
            };

        if (name == "ieee754") {
            // big-endian float из 4 байт
            if (a.size() < 4) { m_error = "ieee754 needs 4 args"; return 0.0; }
            uint32_t raw = ((uint32_t)(uint8_t)arg(0) << 24)
                | ((uint32_t)(uint8_t)arg(1) << 16)
                | ((uint32_t)(uint8_t)arg(2) << 8)
                | (uint32_t)(uint8_t)arg(3);
            float f; std::memcpy(&f, &raw, 4);
            return (double)f;
        }
        if (name == "ieee754le") {
            // little-endian float
            if (a.size() < 4) { m_error = "ieee754le needs 4 args"; return 0.0; }
            uint32_t raw = ((uint32_t)(uint8_t)arg(3) << 24)
                | ((uint32_t)(uint8_t)arg(2) << 16)
                | ((uint32_t)(uint8_t)arg(1) << 8)
                | (uint32_t)(uint8_t)arg(0);
            float f; std::memcpy(&f, &raw, 4);
            return (double)f;
        }
        if (name == "hex2dec")  return (uint8_t)arg(0) * 256.0 + (uint8_t)arg(1);
        if (name == "hex2dec4") return ((uint32_t)(uint8_t)arg(0) << 24)
            | ((uint32_t)(uint8_t)arg(1) << 16)
            | ((uint32_t)(uint8_t)arg(2) << 8)
            | (uint32_t)(uint8_t)arg(3);
        if (name == "abs")   return std::abs(arg(0));
        if (name == "sqrt")  return std::sqrt(arg(0));
        if (name == "round") return std::round(arg(0));
        if (name == "floor") return std::floor(arg(0));
        if (name == "ceil")  return std::ceil(arg(0));
        if (name == "min")   return std::min(arg(0), arg(1));
        if (name == "max")   return std::max(arg(0), arg(1));
        if (name == "pow")   return std::pow(arg(0), arg(1));

        m_error = QString("Unknown function '%1'").arg(name);
        return 0.0;
    }
};