#pragma once
#include "Console.h"

Color Console::type_to_color(ConsoleMessageType type) { // Тип ошибки превращает в цвет
	switch (type) {
	case ConsoleMessageType::DEF:
		return Color(255, 255, 255, 255);
	case ConsoleMessageType::INFO:
		return Color(130, 130, 130, 255);
	case ConsoleMessageType::WARN:
		return Color(255, 230, 0, 255);
	case ConsoleMessageType::ERR:
		return Color(200, 0, 0, 255);
	case ConsoleMessageType::SUCC:
		return Color(0, 170, 0, 255);
	}
}

Console::Console(Vector2u _size) {
	size = _size;
	if (!font.loadFromFile("resources/console/font.ttf")) {
		cout << "Unable to find font.ttf for in-game console!" << endl;
	}
	else if (!console_texture.create(size.x, size.y)) {
		cout << "Unable to create texture for console!" << endl;
	}
	else
		cout << "In-game console created successfully, all further info you can see in it" << endl;
	text.setFont(font);
	text.setCharacterSize(character_size);
	sprite.setTexture(console_texture.getTexture());
	

	// Шрифт должен быть monospaced, ниже идет вычисление средней длины символа шрифта
	string test = "!#$ % &()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	text.setString(test);
	avg_symbol_size.x = offset_koeff.x * float(text.getGlobalBounds().width) / float(test.size());
	avg_symbol_size.y = offset_koeff.y * float(text.getGlobalBounds().height);
	max_symbols_in_line = size.x / avg_symbol_size.x;

	log("Welcome to stEngine console. Today is " + get_date_string() + ", hope you don't encounter any bugs :)");
}

void Console::log(string msg, ConsoleMessageType type) {
	if (messages.size() >= max_console_size)
		messages.pop_front();

	msg = "[" + get_time_string() + "] " + msg;

	// Если текст нового сообщения совпадает с предыдущим сообщением, то не будем их повторять
	if (messages.size() > 0)
		if (msg == messages[messages.size() - 1].first)
			return;

	messages.push_back(pair<string, ConsoleMessageType>(msg, type));
	render();
}

void Console::render() {
	console_texture.clear(background_color);
	int sum_height = size.y, msg_counter = messages.size() - 1;
	int lines = 0;

	while (sum_height > 0 && msg_counter >= 0) {
		text.setFillColor(type_to_color(messages[msg_counter].second));
		for (int i = messages[msg_counter].first.size() / max_symbols_in_line; i >= 0; i--) {

			text.setString(messages[msg_counter].first.substr(i * max_symbols_in_line, max_symbols_in_line));
			lines++;
			sum_height = size.y - lines * avg_symbol_size.y;
			text.setPosition(Vector2f(0, sum_height));
			console_texture.draw(text);
		}
		msg_counter--;
	}
	console_texture.display();
}