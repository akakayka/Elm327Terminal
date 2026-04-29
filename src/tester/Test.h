#pragma once

#include <QObject>
#include <QString>
#include <QList>

// Описание одного диапазона допустимых значений
struct Range {
	int min = 0;
	int max = 0;
};

// Тест — привязан к message id (canId) и содержит набор диапазонов.
// Если указан один диапазон — он применяется ко всем элементам данных.
// Если указано столько диапазонов, столько и элементов данных — применяется попарная проверка.
struct Test {
	int id = 0;                // внутренний id в хранилище
	QString name;              // удобное имя
	QString description;
	int canId = 0;             // идентификатор сообщения (по нему ищется тест)
	QList<Range> ranges;       // диапазоны для проверки

	bool isValid() const {
		return !name.isEmpty() && !ranges.isEmpty();
	}
};

