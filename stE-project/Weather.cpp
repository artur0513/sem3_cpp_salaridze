#pragma once
#include "Weather.h"

Glsl::Vec3 operator*(Glsl::Vec3 vec, float f) {
	return Glsl::Vec3(vec.x * f, vec.y * f, vec.z * f);
}

Glsl::Vec4 operator*(Glsl::Vec4 vec, float f) {
	return Glsl::Vec4(vec.x * f, vec.y * f, vec.z * f, vec.w * f);
}

Glsl::Vec4 operator+(Glsl::Vec4 vec1, Glsl::Vec4 vec2) {
	return Glsl::Vec4(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z, vec1.w + vec2.w);
}

Glsl::Vec3 operator+(Glsl::Vec3 vec1, Glsl::Vec3 vec2) {
	return Glsl::Vec3(vec1.x + vec2.x, vec1.y + vec2.y, vec1.z + vec2.z);
}

// Может стоит добавить другую формулу для shift_koeff, как например smoothstep в glsl
Vector3f Light_cycle::get_light_color(double time) {
	time = fmod(time, 24.0);

	if (light_color.size() == 0)
		return Vector3f(0.f, 0.f, 0.f);
	if (light_color.size() == 1)
		return light_color[0].second;

	// то ли я тупой, то ли проще не написать этот кусок куска ????
	if (time < light_color[0].first) {
		float shift_koeff = (time + 24.0 - light_color[light_color.size() - 1].first) / (light_color[0].first + 24.0 - light_color[light_color.size() - 1].first);
		return shift_koeff * light_color[0].second + (1 - shift_koeff) * light_color[light_color.size() - 1].second;
	}
	if (time > light_color[light_color.size() - 1].first) {
		float shift_koeff = (time - light_color[light_color.size() - 1].first) / (light_color[0].first + 24.0 - light_color[light_color.size() - 1].first);
		return shift_koeff * light_color[0].second + (1 - shift_koeff) * light_color[light_color.size() - 1].second;
	}
	for (int i = 1; i < light_color.size(); i++) {
		if (time < light_color[i].first) {
			float shift_koeff = (time - light_color[i - 1].first) / (light_color[i].first - light_color[i - 1].first);
			return light_color[i].second * shift_koeff + (1 - shift_koeff) * light_color[i - 1].second;
		}
	}

	return light_color[0].second; // На всякий случай
}

void Light_cycle::load_from_file(ifstream& file) {
	int n;
	file >> n; // Количество пар

	for (int i = 0; i < n; i++) {
		Vector3f color;
		double time;
		file >> time >> color.x >> color.y >> color.z;
		light_color.push_back(pair<double, Vector3f>(time, color));
	}

	sort(light_color.begin(), light_color.end(), [](pair<double, Vector3f>& obj1, pair<double, Vector3f>& obj2) {return (obj1.first < obj2.first); });
}

void Weather_state::load_from_file(ifstream& file) {
	file >> weather_name >> probability >> avg_time;

	cycle.load_from_file(file);

	file >> wind_mod >> fog_def >> fog_cloud >> fog_height >> rain >> leaves >> thunder;
	file >> corner_shading_radius >> corner_shading_color.x >> corner_shading_color.y >> corner_shading_color.z >> corner_shading_color.w;

	file >> color_correction_r.x >> color_correction_r.y >> color_correction_r.z;
	file >> color_correction_g.x >> color_correction_g.y >> color_correction_g.z;
	file >> color_correction_b.x >> color_correction_b.y >> color_correction_b.z;

	file >> noise_alpha;
}

void Weather_system::update_leaves() {
	if (leaf_samples.size() == 0)
		return;

	// Здесь обновление координат существующих листиков
	for (auto i = 0; i < leaves.size(); i++) {
		leaves[i].first->world_pos += leaves[i].second * double(leaf_clock.getElapsedTime().asSeconds());
		Vector2d dist = leaves[i].first->world_pos - eng->main_camera.world_pos;
		if (dist.x + dist.y > (eng->main_camera.world_size.x + eng->main_camera.world_size.y) * leaves_delete_koeff) {
			eng->sprite_objects_for_deletion.push_back(leaves[i].first);
			delete leaves[i].first;
			leaves.erase(leaves.begin() + i);
		}
	}
	leaf_clock.restart();

	int target_leaves = (weather_order[0].first->leaves * weather_shift_koeff + weather_order[1].first->leaves * (1.0 - weather_shift_koeff)) *
		eng->main_camera.world_size.x * eng->main_camera.world_size.y * leaves_max_density;
	target_leaves = min(target_leaves, max_leaves);

	if (current_wind_mod < min_wind_mod)
		return;

	// Тут спавн новых листиков (по одному за раз и не чаще чем раз в (cooldown/количество листьев, которые необходимо заспавнить) секунд)
	int need_to_spawn = target_leaves - leaves.size();
	if (need_to_spawn > 0 && leaf_spawn_clock.getElapsedTime() > leaf_spawn_cooldown / float(max(1, need_to_spawn))) {
		Vector2d target_pos;
		target_pos.x = eng->main_camera.world_size.x * double((rand() % 101) - 50) / 51.0 + eng->main_camera.world_pos.x;
		target_pos.y = eng->main_camera.world_size.y * double((rand() % 101) - 50) / 51.0 + eng->main_camera.world_pos.y;

		float angle = wind_angle + double((rand() % 101) - 50) / 50.0 * leaves_angle_delta;
		double leaf_mod_speed = current_wind_mod * (1 + float(rand() % 100) / 500);
		Vector2d speed = Vector2d(cos(angle) * leaf_mod_speed, sin(angle) * leaf_mod_speed);

		Vector2d spawn_pos = target_pos - normalized(speed - eng->main_camera.get_speed()) * (eng->main_camera.world_size.x + eng->main_camera.world_size.y) * leaves_spawn_koeff;
		Sprite_object* leaf = new Sprite_object(leaf_samples[rand() % leaf_samples.size()]);
		leaves.push_back(pair<Sprite_object*, Vector2d>(leaf, speed));
		leaf->world_pos = spawn_pos;
		leaf->current_animation = 0;
		leaf->animations[0].speed_koeff = 1 + float(rand() % 100) / 500;
		eng->sprite_objects.push_back(leaf);
		leaf_spawn_clock.restart();
	}
}

Weather_system::Weather_system(Graphics_engine* _eng, Game_time* _time) {
	eng = _eng;
	game_time = _time;

	last_state_time = double(game_time->total_seconds) / 3600.0;
}

// Прочитать из файла все состояния погоды
void Weather_system::load_from_file(ifstream& file) {
	Clock load_clock;
	int counter;
	file >> counter;

	for (int i = 0; i < counter; i++) {
		Weather_state st;
		st.load_from_file(file);
		states.push_back(st);
		total_probability += st.probability;
	}

	srand(time(0));
	weather_order.push_back(pair<Weather_state*, float>(&states[rand() % counter], 0.0)); // Надо добавить погоду в самое начало для корректной работы

	file >> counter; // Считываем листики
	for (int i = 0; i < counter; i++) {
		Sprite_object leaf;
		leaf.load_from_file(file);
		leaf_samples.push_back(leaf);
	}
	console->log("Weather system configured in " + to_string(load_clock.getElapsedTime().asMilliseconds()) + " milliseconds", ConsoleMessageType::INFO);
	console->log("Loaded " + to_string(states.size()) + " weather states and " + to_string(leaf_samples.size()) + " leaf samples in weather system", ConsoleMessageType::INFO);
}

// Добавить в очередь новые погодные состояния в количестве num_of_states штук
void Weather_system::create_order(int num_of_states) {
	if (states.size() == 0)
		return;

	srand(time(0));
	for (int i = 0; i < num_of_states; i++) {
		int r = rand() % total_probability;
		auto itrtr = states.begin();
		while (r > itrtr->probability) {
			r -= itrtr->probability;
			itrtr++;
		}

		float time = last_state_time + itrtr->avg_time * (float(rand() % 100) / 100.f + 0.5f); // Время предыдущей погоды + от 0.5 до 1.5 среднего времени следующего
		last_state_time = time;
		weather_order.push_back(pair<Weather_state*, float>(&(*itrtr), time));
	}

	console->log(to_string(num_of_states) + " weather events generated and added to the queue ( total in queue " + to_string(weather_order.size()) + " weathers)", ConsoleMessageType::INFO);
}


void Weather_system::update() {
	if (states.size() == 0) {
		return;
	}

	if (weather_order.size() < 5)
		create_order(5);

	while (game_time->total_hours > weather_order[1].second) {
		weather_order.pop_front();
		console->log("Current weather: " + weather_order[0].first->weather_name, ConsoleMessageType::INFO);
	}

	//weather_shift_koeff = 1.0 - (game_time->total_hours - weather_order[0].second) / (weather_order[1].second - weather_order[0].second);
	weather_shift_koeff = 1.0 - smoothstep(weather_order[0].second, weather_order[1].second, float(game_time->total_hours));

	// Погоду можно обновлять каждую игрову секунду, это не должно быть шибко заметноы
	if (last_update_second != game_time->total_seconds) {
		wind_angle += double((rand() % 2) * 2 - 1) * 0.00575958653; // Когда проходит одна секунда, то обновляем направление ветра
		current_wind_mod = weather_order[0].first->wind_mod * weather_shift_koeff + weather_order[1].first->wind_mod * (1.0 - weather_shift_koeff);
		current_wind_mod *= 1 + 0.028 * sin(double(game_time->total_seconds % 1821) / 1821.0 * TWOPI) + 0.056 * sin(sin(double(game_time->total_seconds % 1274) / 1274.0 * TWOPI)) + 0.037 * sin(sin(double(game_time->total_seconds % 2154) / 2154.0 * TWOPI));
		eng->wind_speed = Vector2f(cos(wind_angle) * current_wind_mod, sin(wind_angle) * current_wind_mod);

		// Тут просто ужас, который где то должен быть прописан, смешиваем две погоды
		eng->fog_height = weather_order[0].first->fog_height * weather_shift_koeff + weather_order[1].first->fog_height * (1.0 - weather_shift_koeff);
		eng->fog_cloud = weather_order[0].first->fog_cloud * weather_shift_koeff + weather_order[1].first->fog_cloud * (1.0 - weather_shift_koeff);
		eng->fog_def = weather_order[0].first->fog_def * weather_shift_koeff + weather_order[1].first->fog_def * (1.0 - weather_shift_koeff);
		eng->corner_shading_radius = weather_order[0].first->corner_shading_radius * weather_shift_koeff + weather_order[1].first->corner_shading_radius * (1.0 - weather_shift_koeff);
		eng->noise_alpha = weather_order[0].first->noise_alpha * weather_shift_koeff + weather_order[1].first->noise_alpha * (1.0 - weather_shift_koeff);
		eng->rain = weather_order[0].first->rain * weather_shift_koeff + weather_order[1].first->rain * (1.0 - weather_shift_koeff);

		eng->corner_shading_color = weather_order[0].first->corner_shading_color * weather_shift_koeff + weather_order[1].first->corner_shading_color * (1.0 - weather_shift_koeff);
		eng->color_correction_r = weather_order[0].first->color_correction_r * weather_shift_koeff + weather_order[1].first->color_correction_r * (1.0 - weather_shift_koeff);
		eng->color_correction_g = weather_order[0].first->color_correction_g * weather_shift_koeff + weather_order[1].first->color_correction_g * (1.0 - weather_shift_koeff);
		eng->color_correction_b = weather_order[0].first->color_correction_b * weather_shift_koeff + weather_order[1].first->color_correction_b * (1.0 - weather_shift_koeff);

		eng->ambient_light = weather_order[0].first->cycle.get_light_color(game_time->day_hours) * weather_shift_koeff +
			weather_order[1].first->cycle.get_light_color(game_time->day_hours) * (1.0 - weather_shift_koeff);
	}

	last_update_second = game_time->total_seconds;
	update_leaves();
}