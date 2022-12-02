#pragma once

#include "Common.h"

enum class ConsoleMessageType {
	DEF, // �� ���������
	INFO, // ������ ����������
	WARN, // ��������������
	ERR, // ������
	SUCC, // ��������� �� ������
};

class Console {
private:
	const int max_console_size = 80;
	const Color background_color = Color(0, 0, 0, 70);
	const int character_size = 18;

	Vector2f offset_koeff = Vector2f(1.1f, 1.1f);
	Vector2f avg_symbol_size;
	unsigned max_symbols_in_line;

	Vector2u size;
	Text text;
	RenderTexture console_texture;

	Color type_to_color(ConsoleMessageType type);

	Console(Vector2u _size = Vector2u(1080, 600));
	Console(const Console&);
	Console& operator=(const Console&) {};
	~Console() {};

public:
	Font font;
	deque<pair<string, ConsoleMessageType>> messages; // ������ �� ����� ����������� � �������
	Sprite sprite;

	static Console* get_instance() {
		static Console instance;
		return &instance;
	}

	void log(string msg, ConsoleMessageType type = ConsoleMessageType::DEF); // ��������� ��������� � �������

	void render();
};