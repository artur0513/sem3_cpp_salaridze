#pragma once
#include "Car.h"

Car::Car(gr::Graphics_engine* _eng) {
	if (!font.loadFromFile("resources/console/font.ttf"))
		console->log("Cannot find font for car hud", ConsoleMessageType::ERR);
	
	if (!car_hud.create(Vector2u(256, 256)))
		console->log("Cannot create render texture for car hud", ConsoleMessageType::ERR);

	hud_text.setFont(font);
	hud_text.setFillColor(Color(255, 255, 255, 255));
	tick_time = clock.getElapsedTime();
	eng = _eng;
}

void Car::load_from_file(ifstream& file) {
	file >> collider_pos.x >> collider_pos.y >> collider_size.x >> collider_size.y >> max_fuel;
	fuel = max_fuel;
	
	int n;
	file >> n;
	shift_koef.resize(n+1);
	shift_koef[0] = 0;
	for (int i = 0; i < n; i++) { // Передачи
		file >> shift_koef[i + 1];
	}

	file >> n;
	power.resize(n);
	for (int i = 0; i < n; i++) { // Мощность
		file >> power[i].first >> power[i].second;
	}

	sprite = new gr::Sprite_object;
	sprite->load_from_file(file);
	for (int i = 0; i < 8; i++) {
		gr::Light_source* src = new gr::Light_source;
		src->load_from_file(file);
		src->active = false;
		lights.push_back(src);
		eng->lights.push_back(src);
	}
	
	eng->sprite_objects.push_back(sprite);
}

void Car::update_pos(Vector2d delta_move) {
	delta_move.y = clamp(sprite->world_pos.y + delta_move.y, 0.0, 1.5) - sprite->world_pos.y;
	
	sprite->world_pos += delta_move;
	for (int i = 0; i < 8; i++) {
		lights[i]->world_pos += delta_move;
	}
	collider_pos += delta_move;
	pos += delta_move;
	distance_travel += delta_move.x;
}

void Car::input(sf::Event event) {

	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::W)
			throtle = true;

		if (event.key.code == sf::Keyboard::S)
			brake = true;

		if (event.key.code == sf::Keyboard::A)
			turn_up = true;

		if (event.key.code == sf::Keyboard::D)
			turn_down = true;

		if (event.key.code == sf::Keyboard::B)
			starter = true;

		if (event.key.code == sf::Keyboard::L)
			front_lights = !front_lights;

		if (event.key.code == sf::Keyboard::Up && gear < shift_koef.size() - 1)
			gear++;

		if (event.key.code == sf::Keyboard::Down && gear > 0)
			gear--;
	}


	if (event.type == sf::Event::KeyReleased) {
		if (event.key.code == sf::Keyboard::W)
			throtle = false;

		if (event.key.code == sf::Keyboard::S)
			brake = false;

		if (event.key.code == sf::Keyboard::A)
			turn_up = false;

		if (event.key.code == sf::Keyboard::D)
			turn_down = false;

		if (event.key.code == sf::Keyboard::B)
			starter = false;
	}

}

double Car::get_force() {

	double cheat = 0.0;
	if (gear == 1)
		cheat = max(double(3000 - rpm) / 90.0, 0.0);

	if (rpm < power[0].second) {
		rpm = 0;
		engine_on = false;
		return 0.0;
	}
	for (int i = 1; i < power.size(); i++) {
		if (rpm < power[i].second) {
			return (double(rpm - power[i-1].second) * power[i].first + double(power[i].second - rpm) * power[i-1].first) / double(power[i].second - power[i-1].second)*power_boost + cheat;
		}
	}
	return 0.0;
}

void Car::update() {
	eng->color_correction_r = Glsl::Vec3(1.0 + clamp(1.0 - hp / 100.0, 0.0, 1.0), 0.0, 0.0);
	eng->corner_shading_color = Glsl::Vec4(1.0, 0.0, 0.0, 0.75);
	eng->corner_shading_radius = clamp(hp / 100.0, 0.0, 0.8);
	eng->aberration_radius = 1.0 - 0.75 * (1.0 - fuel/max_fuel);
	eng->aberration_shift = 0.1;

	if (fuel < 0.0) {
		engine_on = false;
		fuel = 0.0;
	}
	if (fuel > max_fuel)
		fuel = max_fuel;
	hp = min(hp, 200.0);

	speed.y = 0.0;
	if (turn_up)
		speed.y += 2.5 * clamp(speed.x * speed.x / 25.0, 0.0, 1.0);
	if (turn_down)
		speed.y -= 2.5 * clamp(speed.x * speed.x / 25.0, 0.0, 1.0);

	boost_time -= (clock.getElapsedTime() - tick_time);
	if (boost_time < sf::seconds(0))
		power_boost = 1.0;

	penalty_time -= (clock.getElapsedTime() - tick_time);
	if (penalty_time < sf::seconds(0))
		penalty = 1.0;

	Vector2d delta_move = Vector2d(0.0, 0.0);
	speed.x += acceleration.x * (clock.getElapsedTime() - tick_time).asSeconds();
	speed.x = max(speed.x, 0.0);
	delta_move.x = speed.x * (clock.getElapsedTime() - tick_time).asSeconds();
	delta_move.y = speed.y * (clock.getElapsedTime() - tick_time).asSeconds();
	
	if (engine_on) {
		fuel -= (clock.getElapsedTime() - tick_time).asSeconds() * 0.66;
		fuel -= delta_move.x;
	}
	update_pos(delta_move);

	if (speed.x < 0.0)
		speed.x = 0.0;
	
	rpm = speed.x * shift_koef[gear] * 270;
	if (rpm < def_rpm && engine_on) {
		clutch_koef = clamp(double(rpm) / double(def_rpm), 0.25, 1.0);
		rpm = def_rpm;
	}
	
	if (starter) {
		engine_on = !engine_on;
		if (engine_on)
			rpm = def_rpm;
		else
			rpm = 0;
		starter = false;
	}

	acceleration.x = - 0.0043 * speed.x * speed.x * penalty - max(double(rpm - 4000) / 6000.0, 0.0) - 0.03;
	if (throtle) {
		if (gear != 0 && engine_on) {
			acceleration.x += ((get_force() * shift_koef[gear])) * clutch_koef / 112.0;
		}
	}
	if (brake) {
		if (speed.x > 4.0)
			acceleration.x -= 2.5;
		else if (speed.x < 4.0 && speed.x > 0.0)
			acceleration.x -= 0.6 * (speed.x + 2.0);
		else {
			acceleration.x = 0.0;
		}
	}

	// Логика работы задних фар
	if (brake && engine_on) {
		lights[0]->active = false;
		lights[1]->active = false;
		lights[4]->active = true;
		lights[5]->active = true;
	} else if (!brake && engine_on) {
		lights[0]->active = true;
		lights[1]->active = true;
		lights[4]->active = false;
		lights[5]->active = false;
	} else {
		lights[0]->active = false;
		lights[1]->active = false;
		lights[4]->active = false;
		lights[5]->active = false;
	}

	//Логика работы передних фар
	if (engine_on && !front_lights) {
		lights[2]->active = true;
		lights[3]->active = true;
		lights[6]->active = false;
		lights[7]->active = false;
	} else if (engine_on && front_lights) {
		lights[2]->active = false;
		lights[3]->active = false;
		lights[6]->active = true;
		lights[7]->active = true;
	} else {
		lights[2]->active = false;
		lights[3]->active = false;
		lights[6]->active = false;
		lights[7]->active = false;
	}

	car_hud.clear(Color(0, 0, 0, 0));
	hud_text.setPosition(Vector2f(0, 30));
	hud_text.setString("rpm: " + to_string(rpm));
	if (clutch_koef < 0.9)
		hud_text.setFillColor(Color(255, 127, 0));
	car_hud.draw(hud_text);
	hud_text.setFillColor(Color(255, 255, 255));

	hud_text.setPosition(Vector2f(0, 50));
	hud_text.setString("gear: " + to_string(gear));
	car_hud.draw(hud_text);

	hud_text.setPosition(Vector2f(0, 70));
	hud_text.setString("speed: " + to_string(int(speed.x * 9.0)) + " km/h");
	car_hud.draw(hud_text);

	hud_text.setPosition(Vector2f(0, 90));
	hud_text.setString("hp: " + to_string(int(hp)));
	if (hp/100.0 < 0.25)
		hud_text.setFillColor(Color(255, 0, 0));
	car_hud.draw(hud_text);
	hud_text.setFillColor(Color(255, 255, 255));

	hud_text.setPosition(Vector2f(0, 110));
	hud_text.setString("fuel: " + to_string(int(fuel)));
	if (fuel/max_fuel < 0.25)
		hud_text.setFillColor(Color(255, 0, 0));
	car_hud.draw(hud_text);
	hud_text.setFillColor(Color(255, 255, 255));

	hud_text.setPosition(Vector2f(0, 130));
	hud_text.setString("power boost " + to_string(int(power_boost * 100.0 - 100.0)));
	if (power_boost > 1.01){
		hud_text.setFillColor(Color(127, 255, 127));
		car_hud.draw(hud_text);
	}
	else if (power_boost < 0.99) {
		hud_text.setFillColor(Color(255, 127, 127));
		car_hud.draw(hud_text);
	}
	hud_text.setFillColor(Color(255, 255, 255));
	car_hud.display();

	tick_time = clock.getElapsedTime();
}