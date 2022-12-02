#pragma once
#include "Common.h"
#include "Graphics.h"
#include "Car.h"

class Static_object {
private:
	vector<gr::Sprite_object*> diffuse;
	vector<gr::Sprite_object*> height;
	vector<gr::Light_source*> lights;
	vector<gr::Effect*> effects;

	gr::Graphics_engine* eng;
public:
	Static_object(ifstream& file, gr::Graphics_engine* _eng);

	Static_object(std::string filename, gr::Graphics_engine* _eng);

	Static_object(const Static_object& obj);

	Static_object& operator=(const Static_object& obj);

	Static_object(Static_object&& obj);

	Static_object& operator=(Static_object&& obj);

	~Static_object();

	void move(Vector2d vec);

	double get_max_x();

	int get_total_size();
};


class Obstacle {
private:
	Vector2d pos, size;

public:
	Console* console;
	Static_object obj;

	Obstacle(ifstream& file, gr::Graphics_engine* _eng);

	virtual void apply_effect(Car& car);

	bool check_restriction(Car& car);

	void move(Vector2d vec);

	void set_pos(Vector2d vec);
};


class Gas_can : public Obstacle {
public:
	Gas_can(ifstream& file, gr::Graphics_engine* _eng);

	void apply_effect(Car& car) override;
};

class Boost : public Obstacle {
public:
	Boost(ifstream& file, gr::Graphics_engine* _eng);

	void apply_effect(Car& car) override;
};

class Barrier : public Obstacle {
public:
	Barrier(ifstream& file, gr::Graphics_engine* _eng);

	void apply_effect(Car& car) override;
};

class Repair_kit : public Obstacle {
public:
	Repair_kit(ifstream& file, gr::Graphics_engine* _eng);

	void apply_effect(Car& car) override;
};


class LevelGenerator {
private:
	Console* console = Console::get_instance();

	Car* car;

	vector<Static_object> road;
	vector<Static_object> ground;
	vector<Static_object> lamppost;

	vector<Static_object> decor_upper;
	vector<Static_object> decor_lower;

	vector<Obstacle*> obsts;

	double road_length = 30.8;
	double ground_length = 30.0;
	double lamppost_length = 20.0;
	double decor_length = 10.0;

	double decor_total_length = 0.0;
	gr::Graphics_engine* eng;
	vector<Static_object*> decor_to_move;

	void move_decor(vector<Static_object>& decor);
public:
	LevelGenerator(std::string filename, gr::Graphics_engine* _eng, Car &_car);

	void update();
};