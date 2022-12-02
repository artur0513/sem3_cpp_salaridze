#include "LevelGenerator.h"

Static_object::Static_object(std::string filename, gr::Graphics_engine* _eng) {
	eng = _eng;
	ifstream file(filename);
	if (!file.is_open())
		return;

	char t;
	while (file >> t) {
		if (t == 's') {
			gr::Sprite_object* obj = new gr::Sprite_object;
			obj->load_from_file(file);
			eng->sprite_objects.push_back(obj);
			diffuse.push_back(obj);
		}
		if (t == 'h') {
			gr::Sprite_object* obj = new gr::Sprite_object;
			obj->load_from_file(file);
			eng->height_objects.push_back(obj);
			height.push_back(obj);
		}
		if (t == 'l') {
			gr::Light_source* src = new gr::Light_source;
			src->load_from_file(file);
			eng->lights.push_back(src);
			lights.push_back(src);
		}
		if (t == 'e') {
			gr::Effect* eff = new gr::Effect;
			eff->load_from_file(file);
			eff->effect_temp = &eng->effect_temp;
			eng->effects.push_back(eff);
			effects.push_back(eff);
		}
	}
}

Static_object::Static_object(ifstream &file, gr::Graphics_engine* _eng) {
	eng = _eng;

	char t;
	while (file >> t) {
		if (t == 's') {
			gr::Sprite_object* obj = new gr::Sprite_object;
			obj->load_from_file(file);
			eng->sprite_objects.push_back(obj);
			diffuse.push_back(obj);
		}
		if (t == 'h') {
			gr::Sprite_object* obj = new gr::Sprite_object;
			obj->load_from_file(file);
			eng->height_objects.push_back(obj);
			height.push_back(obj);
		}
		if (t == 'l') {
			gr::Light_source* src = new gr::Light_source;
			src->load_from_file(file);
			eng->lights.push_back(src);
			lights.push_back(src);
		}
		if (t == 'e') {
			gr::Effect* eff = new gr::Effect;
			eff->load_from_file(file);
			eff->effect_temp = &eng->effect_temp;
			eng->effects.push_back(eff);
			effects.push_back(eff);
		}
	}
}

Static_object::Static_object(const Static_object& obj) {
	eng = obj.eng;
	for (int i = 0; i < obj.diffuse.size(); i++) {
		gr::Sprite_object* sobj = new gr::Sprite_object(*obj.diffuse[i]);
		eng->sprite_objects.push_back(sobj);
		diffuse.push_back(sobj);
	}
	for (int i = 0; i < obj.height.size(); i++) {
		gr::Sprite_object* sobj = new gr::Sprite_object(*obj.height[i]);
		eng->height_objects.push_back(sobj);
		height.push_back(sobj);
	}
	for (int i = 0; i < obj.lights.size(); i++) {
		gr::Light_source* sobj = new gr::Light_source(*obj.lights[i]);
		eng->lights.push_back(sobj);
		lights.push_back(sobj);
	}
	for (int i = 0; i < obj.effects.size(); i++) {
		gr::Effect* sobj = new gr::Effect(*obj.effects[i]);
		eng->effects.push_back(sobj);
		effects.push_back(sobj);
	}
}

Static_object& Static_object::operator=(const Static_object& obj) {
	diffuse.clear();
	height.clear();
	lights.clear();
	effects.clear();
	for (int i = 0; i < obj.diffuse.size(); i++) {
		gr::Sprite_object* sobj = new gr::Sprite_object(*obj.diffuse[i]);
		eng->sprite_objects.push_back(sobj);
		diffuse.push_back(sobj);
	}
	for (int i = 0; i < obj.height.size(); i++) {
		gr::Sprite_object* sobj = new gr::Sprite_object(*obj.height[i]);
		eng->height_objects.push_back(sobj);
		height.push_back(sobj);
	}
	for (int i = 0; i < obj.lights.size(); i++) {
		gr::Light_source* sobj = new gr::Light_source(*obj.lights[i]);
		eng->lights.push_back(sobj);
		lights.push_back(sobj);
	}
	for (int i = 0; i < obj.effects.size(); i++) {
		gr::Effect* sobj = new gr::Effect(*obj.effects[i]);
		eng->effects.push_back(sobj);
		effects.push_back(sobj);
	}
	return *this;
}

Static_object::Static_object(Static_object&& obj) {
	eng = obj.eng;

	for (int i = 0; i < obj.diffuse.size(); i++)
		diffuse.push_back(obj.diffuse[i]);

	for (int i = 0; i < obj.height.size(); i++) 
		height.push_back(obj.height[i]);
	
	for (int i = 0; i < obj.lights.size(); i++)
		lights.push_back(obj.lights[i]);
	
	for (int i = 0; i < obj.effects.size(); i++)
		effects.push_back(obj.effects[i]);
	
	obj.diffuse.clear();
	obj.height.clear();
	obj.effects.clear();
	obj.lights.clear();
}

Static_object& Static_object::operator=(Static_object&& obj) {
	eng = obj.eng;

	for (int i = 0; i < obj.diffuse.size(); i++)
		diffuse.push_back(obj.diffuse[i]);

	for (int i = 0; i < obj.height.size(); i++)
		height.push_back(obj.height[i]);

	for (int i = 0; i < obj.lights.size(); i++)
		lights.push_back(obj.lights[i]);

	for (int i = 0; i < obj.effects.size(); i++)
		effects.push_back(obj.effects[i]);

	obj.diffuse.clear();
	obj.height.clear();
	obj.effects.clear();
	obj.lights.clear();
	return *this;
}

Static_object::~Static_object() {}; // Всю графическую фигню удалит графический движок, мы ничего не делаем

void Static_object::move(Vector2d vec) {
	for (int i = 0; i < diffuse.size(); i++)
		diffuse[i]->world_pos += vec;

	for (int i = 0; i < height.size(); i++)
		height[i]->world_pos += vec;

	for (int i = 0; i < lights.size(); i++)
		lights[i]->world_pos += vec;

	for (int i = 0; i < effects.size(); i++)
		effects[i]->obj.world_pos += vec;
}

double Static_object::get_max_x() {
	double max_x = 0.0, sprite_x;
	double shift = 3.0;
	for (int i = 0; i < diffuse.size(); i++) {
		sprite_x = diffuse[i]->world_pos.x + diffuse[i]->world_size.x;
		if (sprite_x > max_x) {
			max_x = sprite_x;
		}
	}
	return max_x + shift;
}

int Static_object::get_total_size() {
	return diffuse.size() + height.size() + effects.size() + lights.size();
}


Obstacle::Obstacle(ifstream& file, gr::Graphics_engine* _eng) : obj(file, _eng) {
	file >> pos.x >> pos.y >> size.x >> size.y;
	console = Console::get_instance();
}

void Obstacle::move(Vector2d vec) {
	obj.move(vec);
	pos += vec;
}

void Obstacle::set_pos(Vector2d vec) {
	obj.move(vec - pos);
	pos = vec;
}

void Obstacle::apply_effect(Car& car){}

bool Obstacle::check_restriction(Car& car) {
	return !(car.collider_pos.y < pos.y - size.y || car.collider_pos.y - car.collider_size.y > pos.y || car.collider_pos.x + car.collider_size.x < pos.x || car.collider_pos.x > pos.x + size.x);
}

void Gas_can::apply_effect(Car& car) {
	console->log("Gas can hit", ConsoleMessageType::INFO);
	car.fuel += car.max_fuel*0.14;
}

Gas_can::Gas_can(ifstream& file, gr::Graphics_engine* _eng) : Obstacle(file, _eng) {};

void Boost::apply_effect(Car& car) {
	console->log("Booster hit", ConsoleMessageType::INFO);
	double new_power_boost = 1.1 + double(rand() % 30) / 100.0;
	if (new_power_boost > car.power_boost && car.power_boost > 0.99)
		car.power_boost = new_power_boost;
	car.boost_time = sf::seconds(5.0);
}

Boost::Boost(ifstream& file, gr::Graphics_engine* _eng) : Obstacle(file, _eng) {};

Barrier::Barrier(ifstream& file, gr::Graphics_engine* _eng) : Obstacle(file, _eng) {};

void Barrier::apply_effect(Car& car) {
	console->log("Barrier hit", ConsoleMessageType::INFO);
	car.hp -= 4 * car.speed.x + 10.0;
	car.power_boost = 0.9 - double(rand() % 30) / 100.0;
	car.boost_time = sf::seconds(1.5);

	car.penalty = 1.7;
	car.penalty_time = sf::seconds(1.5);

}

Repair_kit::Repair_kit(ifstream& file, gr::Graphics_engine* _eng) : Obstacle(file, _eng) {};

void Repair_kit::apply_effect(Car& car) {
	console->log("Repair kit hit", ConsoleMessageType::INFO);
	car.hp += 20.0;
}

LevelGenerator::LevelGenerator(std::string filename, gr::Graphics_engine *_eng, Car &_car) {
	ifstream file(filename);
	if (!file.is_open())
		return;

	string objname;
	eng = _eng;
	car = &_car;

	file >> objname;
	Static_object obj_road(objname, eng);
	road.push_back(obj_road);
	obj_road.move(Vector2d(road_length, 0.0));
	road.push_back(obj_road);

	file >> objname;
	Static_object obj_grass(objname, eng);
	ground.push_back(obj_grass);
	obj_grass.move(Vector2d(ground_length, 0.0));
	ground.push_back(obj_grass);

	file >> objname;
	Static_object obj_lamppost(objname, eng);
	lamppost.push_back(obj_lamppost);
	obj_lamppost.move(Vector2d(lamppost_length, 0.0));
	lamppost.push_back(obj_lamppost);
	int n;
	file >> n;
	for (int i = 0; i < n; i++) {
		file >> objname;
		Static_object obj_decor(objname, eng);
		obj_decor.move(Vector2d(decor_length * double(i - 2*i*(i % 2)) / 2.0, 0.0));
		decor_upper.push_back(obj_decor);
		obj_decor.move(Vector2d(2.5 * decor_length, -6.0));
		decor_lower.push_back(obj_decor);
	}

	decor_total_length = decor_length * decor_upper.size() / 2;

	ifstream file1("config/objects/gas_can.txt");
	Obstacle* gas = new Gas_can(file1, eng);
	gas->move(Vector2d(rand()%100 + 50, 0.0));
	obsts.push_back(gas);

	ifstream file2("config/objects/boost.txt");
	Obstacle* boost = new Boost(file2, eng);
	boost->move(Vector2d(rand() % 50 + 25, 0.0));
	obsts.push_back(boost);

	ifstream file3("config/objects/barrier.txt");
	Obstacle* barrier = new Barrier(file3, eng);
	barrier->move(Vector2d(rand() % 50 + 25, 0.0));
	obsts.push_back(barrier);

	ifstream file5("config/objects/barrier.txt");
	Obstacle* barrier2 = new Barrier(file5, eng);
	barrier2->move(Vector2d(rand() % 50 + 450, 0.0));
	obsts.push_back(barrier2);

	ifstream file4("config/objects/repair_kit.txt");
	Obstacle* kit = new Repair_kit(file4, eng);
	kit->move(Vector2d(rand() % 50 + 200, 0.0));
	obsts.push_back(kit);
	
}

void LevelGenerator::move_decor(vector<Static_object>& decor) {
	int decor_forvard = 0;
	double max_decor_x = -100.0;

	decor_to_move.clear();
	for (int i = 0; i < decor.size(); i++) {
		if (decor[i].get_max_x() < eng->main_camera.corner_pos.x) {
			decor_to_move.push_back(&decor[i]);
		}
		else if (decor[i].get_max_x() > eng->main_camera.corner_pos.x + eng->main_camera.world_size.x) {
			decor_forvard++;
		}

		if (decor[i].get_max_x() > max_decor_x)
			max_decor_x = decor[i].get_max_x();
	}
	if (decor_to_move.size() > 0 && decor_forvard < 2) {
		int id = rand() % decor_to_move.size();
		double move_dist = max_decor_x - decor_to_move[id]->get_max_x();
		if (max_decor_x < eng->main_camera.corner_pos.x + eng->main_camera.world_size.x)
			move_dist = eng->main_camera.corner_pos.x + eng->main_camera.world_size.x - decor_to_move[id]->get_max_x();
		decor_to_move[id]->move(Vector2d(move_dist + decor_length*double(100 + rand()%50)/100.0, 0.0));
	}

}

void LevelGenerator::update() {
	//console->log(to_string(road[0].get_max_x()) + "  " + to_string(eng->main_camera.corner_pos.x));
	if (road[0].get_max_x() < eng->main_camera.corner_pos.x) {
		road[0].move(Vector2d(2*road_length, 0.0));
		std::swap(road[0], road[1]);
	}

	if (ground[0].get_max_x() < eng->main_camera.corner_pos.x) {
		ground[0].move(Vector2d(2 * ground_length, 0.0));
		std::swap(ground[0], ground[1]);
	}

	if (lamppost[0].get_max_x() < eng->main_camera.corner_pos.x) {
		lamppost[0].move(Vector2d(2 * lamppost_length, 0.0));
		std::swap(lamppost[0], lamppost[1]);
	}

	move_decor(decor_upper);
	move_decor(decor_lower);

	for (int i = 0; i < obsts.size(); i++) {
		if (obsts[i]->obj.get_max_x() < eng->main_camera.corner_pos.x)
			obsts[i]->set_pos(Vector2d(eng->main_camera.corner_pos.x + eng->main_camera.world_size.x + rand() % 70, 1.40 - double(rand() % 140) / 55.0));

		if (obsts[i]->check_restriction(*car)) {
			obsts[i]->apply_effect(*car);
			obsts[i]->set_pos(Vector2d(eng->main_camera.corner_pos.x + eng->main_camera.world_size.x + rand() % 70, 1.40 - double(rand() % 140) / 55.0));
		}
	}
}