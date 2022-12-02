#pragma once
#include "Common.h"
#include "Graphics.h"

class Car {
private:
	Console* console = Console::get_instance();
	gr::Graphics_engine* eng;

	//vector<gr::Sprite_object> sprites;
	gr::Sprite_object* sprite;
	vector<gr::Light_source*> lights;
	//vector<gr::Effect> effects;

	bool brake = false; // Давим по тормозам
	bool throtle = false; // Давим по газам
	bool turn_down = false;
	bool turn_up = false;
	bool starter = false;
	bool front_lights = false; // Дальний свет
	bool engine_on = false;
	
	double clutch_koef = 0.0;

	Clock clock;
	Time tick_time;

	int rpm = 0, def_rpm = 800;
	vector<pair<double, int>> power; // мощность - обороты
	vector<double> shift_koef; // передаточные отношения

	unsigned short gear = 0; // Передача

	Font font;
	Text hud_text;

	double get_force();

public:
	double distance_travel = 0.0;
	double fuel, max_fuel; // Литров в баке
	double hp = 100.0;

	double power_boost = 1.0;
	Time boost_time = sf::seconds(0.0);

	double penalty = 1.0;
	Time penalty_time = sf::seconds(0.0);

	Vector2d collider_pos;
	Vector2d collider_size;
	RenderTexture car_hud;
	Vector2d pos = Vector2d(0.0, 0.0), speed = Vector2d(0.0, 0.0), acceleration = Vector2d(0.0, 0.0); // Координаты (центра), скорость, ускорение

	Car(gr::Graphics_engine* _eng);

	void update_pos(Vector2d delta_move);

	void load_from_file(ifstream& file);

	void update();

	void input(sf::Event event);
};
