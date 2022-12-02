#pragma once
#include "Graphics.h"
#include "simple-opengl-loader.h"
#include <filesystem>

void gr::Camera::update() {
	tick_count++;
	aspect_ratio = world_size.x / world_size.y;

	corner_pos.x = world_pos.x - world_size.x / 2;
	corner_pos.y = world_pos.y + world_size.y / 2;

	world_rect.left = corner_pos.x;
	world_rect.top = corner_pos.y - world_size.y;
	//world_rect.top = corner_pos.y;
	world_rect.width = world_size.x;
	world_rect.height = world_size.y;

	if (tick_count % 100 == 0) {
		camera_speed = (world_pos - prev_pos) / double(clock.getElapsedTime().asSeconds());
		//console->log(to_string(camera_speed.x) + "  " + to_string(camera_speed.y));
		prev_pos = world_pos;
		clock.restart();
	}
}

Vector2f gr::Camera::world_to_camera_pos(Vector2d gl_pos) {
	Vector2f c_pos;
	c_pos.x = (gl_pos.x - corner_pos.x) * resolution.x / world_size.x;
	c_pos.y = (corner_pos.y - gl_pos.y) * resolution.y / world_size.y;
	return c_pos;
}

Vector2d gr::Camera::camera_to_world_pos(Vector2f c_pos) {
	Vector2d gl_pos;
	gl_pos.x = corner_pos.x + (c_pos.x * world_size.x / resolution.x);
	gl_pos.y = corner_pos.y - (c_pos.y * world_size.y / resolution.y);
	return gl_pos;
}

Vector2f gr::Camera::world_to_glsl_pos(Vector2d gl_pos) {
	Vector2f glsl_pos;
	glsl_pos.x = (gl_pos.x - corner_pos.x) / world_size.x;
	glsl_pos.y = (gl_pos.y - corner_pos.y + world_pos.y) / world_size.y;
	return glsl_pos;
}

Vector2d gr::Camera::get_speed() {
	return camera_speed;
}


template<> bool gr::Resource_manager<Texture>::load_object_from_file(Texture* new_texture, string filename) {
	return new_texture->loadFromFile(filename);
}

template<> bool gr::Resource_manager<Shader>::load_object_from_file(Shader* new_shader, string filename) {
	return new_shader->loadFromFile(filename, Shader::Fragment);
}


void gr::Animation::add_frame(Sprite _sprite, Time _time) { // Добавляем кадр
	number_of_frames += 1;
	full_animation_time += _time;
	frames.push_back(pair<Sprite, Time>(_sprite, full_animation_time));
}

Sprite* gr::Animation::get_frame(Time _time) { // Возвращает указатель на текущий спрайт в зависимости от времени
	_time %= full_animation_time * speed_koeff;
	for (unsigned short i = 0; i < number_of_frames; i++) { // Здесь можно поставить бин поиск для небольшого выигрыша в производительности
		if (_time < frames[i].second * speed_koeff) {
			return &frames[i].first; // как только нашли кадр, время до которого больше нужного, возвращаем
		}
	}
	return &frames[0].first; // на всякий случай вернем какой нибудь кадр если ничего не нашли(хотя по идее это невозможно?)
}

Time gr::Animation::get_frame_time(unsigned short i) {
	if (i < number_of_frames)
		return frames[i].second;
	else
		return milliseconds(0);
}

IntRect gr::Animation::get_texture_rect(unsigned short i) {
	if (i < number_of_frames)
		return frames[i].first.getTextureRect();
	else
		return IntRect(Vector2i(0, 0), Vector2i(0, 0));
}

bool gr::Sprite_object::var_update(Camera& cam) {
	// Проверяем видимость обьекта
	DoubleRect check_rect = cam.world_rect;
	check_rect.height += world_size.y;
	check_rect.left -= world_size.x;
	check_rect.width += world_size.x;
	//check_rect.top += world_size.y;

	//check_rect.contains(world_pos)
	if (!(world_pos.y < cam.corner_pos.y - cam.world_size.y || world_pos.y - world_size.y > cam.corner_pos.y || world_pos.x + world_size.x < cam.corner_pos.x || world_pos.x > cam.corner_pos.x + cam.world_size.x)) { // ПРОВЕРКА ОПЯТЬ НЕ РАБОТАЕТ
		visible = true;
		animation_update();
	}
	else {
		visible = false;
		return false; // Если обьект невидим, то больше ничего не считаем
	}

	// Устанавливаем экранные координаты
	current_sprite->setPosition(cam.world_to_camera_pos(world_pos));

	// Устанавливаем нужный масштаб
	Vector2f new_scale;
	new_scale.x = world_size.x * cam.resolution.x / (cam.world_size.x * pixel_local_size.x);
	new_scale.y = world_size.y * cam.resolution.y / (cam.world_size.y * pixel_local_size.y);
	current_sprite->setScale(new_scale);

	// Пересчитываем слой отрисовки
	layer = world_pos.y - world_size.y + delta_layer;
	return true;
}

// Обновление анимации обьекта
void gr::Sprite_object::animation_update() {
	if (current_animation == -1) {
		current_sprite = &def_sprite;
		return;
	}
	current_sprite = animations[current_animation].get_frame(animation_clock.getElapsedTime());
}

// Считывает спрайт из файла
void gr::Sprite_object::load_from_file(ifstream& file) {
	Resource_manager<Texture>* mng = Resource_manager<Texture>::get_instance();

	bool reading_texture = false;
	char buff;

	texture_name.clear(); // На всякий случай очистим имя текстуры перед считыванием
	// Считывем имя текстуры в кавычках
	while (file >> buff) {
		if (buff == '"') {
			if (reading_texture)
				break;
			else {
				reading_texture = true;
				continue;
			}
		}
		if (reading_texture)
			texture_name.push_back(buff);
	}
	// Когда считали навзвание текстуры, получаем ссылку на текстуру от менеджера
	texture_name = relative(texture_name).string();
	Texture* texture = mng->get_object(texture_name);

	IntRect rect; // для Texture Rect
	int anims, frames_in_anim, frame_time; // Для считывания текстур
	file >> rect.left >> rect.top >> rect.width >> rect.height >> world_pos.x >> world_pos.y >> world_size.x >> world_size.y >> delta_layer >> anims;

	def_sprite.setTexture(*texture);
	def_sprite.setTextureRect(rect);
	def_sprite.setPosition(Vector2f(10000.f, 10000.f)); // КОСТЫЛЬ ДЛЯ ФИКСА БАГА С КАРТОЙ ВЫСОТ

	pixel_local_size.x = def_sprite.getLocalBounds().width;
	pixel_local_size.y = def_sprite.getLocalBounds().height;

	// Считываем анимации
	for (int j = 0; j < anims; j++) {
		Animation anim;
		animations.push_back(anim);

		file >> frames_in_anim;
		for (int k = 0; k < frames_in_anim; k++) {
			file >> rect.left >> rect.top >> rect.width >> rect.height >> frame_time;

			Sprite anim_sprite;
			anim_sprite.setTexture(*texture);
			anim_sprite.setTextureRect(rect);
			animations[j].add_frame(anim_sprite, milliseconds(frame_time));
		}
	}
	current_sprite = &def_sprite;
}

// Сохранение спрайта в файл
void gr::Sprite_object::save_to_file(ofstream& file) {
	IntRect rect = def_sprite.getTextureRect();
	unsigned short anims = animations.size();
	file << "\"" << texture_name << "\" " << rect.left << " " << rect.top << " " << rect.width << " " << rect.height << endl;
	file << world_pos.x << " " << world_pos.y << " " << world_size.x << " " << world_size.y << " " << delta_layer << endl;

	file << anims << endl;
	for (unsigned short i = 0; i < anims; i++) {
		unsigned short frames = animations[i].number_of_frames;
		file << frames << endl;
		for (unsigned short j = 0; j < frames; j++) {
			rect = animations[i].get_texture_rect(j);
			file << rect.left << " " << rect.top << " " << rect.width << " " << rect.height << " " << animations[i].get_frame_time(j).asMilliseconds() << endl;
		}
	}
}

// Вывести короткую информацию о спрайте в консоль
void gr::Sprite_object::print_info() {
	cout << " === Sprite Object Info === " << endl;
	cout << "Sprite texture: " << texture_name << endl;
	cout << "World position: " << world_pos << "  World size: " << world_size << "  Layer delta: " << delta_layer << endl;
	cout << "Current scale: " << current_sprite->getScale() << endl;
	cout << "Is visible now: " << visible << endl;
	cout << "Position on RenderTexture (in pixels): " << current_sprite->getPosition() << endl;
	cout << "Default sprite rect: " << def_sprite.getTextureRect() << endl;
	for (int i = 0; i < animations.size(); i++) {
		cout << "Animation #" << i << " consists of " << animations[i].number_of_frames << " frames, total animation time: " << animations[i].full_animation_time.asMilliseconds() << endl;
	}
}

Sprite* gr::Sprite_object::get_sprite() {
	return current_sprite;
}

double gr::Sprite_object::get_layer() {
	return layer;
}

void gr::Sprite_object::set_delta_layer(double _delta_layer) {
	delta_layer = _delta_layer;
}

string gr::Sprite_object::get_texture_name() {
	return texture_name;
}

void gr::Effect::load_from_file(ifstream& file) {
	Resource_manager<Shader>* mng = Resource_manager<Shader>::get_instance();
	obj.load_from_file(file); // Просто загружаем спрайт на основе которого строится эффект

	bool reading_shader = false;
	char buff;

	shader_name.clear(); // На всякий случай очистим имя текстуры перед считыванием
	// Считывем имя текстуры в кавычках
	while (file >> buff) {
		if (buff == '"') {
			if (reading_shader)
				break;
			else {
				reading_shader = true;
				continue;
			}
		}
		if (reading_shader)
			shader_name.push_back(buff);
	}
	// Когда считали навзвание текстуры, получаем ссылку на текстуру от менеджера
	shader_name = relative(shader_name).string();
	shader = mng->get_object(shader_name); // Получаем указатель на шейдер

	short attrs; // Получаем количество аттрибутов
	file >> attrs;
	for (short i = 0; i < attrs; i++) { // И считываем пары из названия переменной и значения
		string s_temp;
		float f_temp;
		file >> s_temp >> f_temp;
		shader_attrs.push_back(pair<string, float>(s_temp, f_temp));
	}
}

// Создание спрайта эффекта на основе текстуры на которую эффект будет накладываться
void gr::Effect::draw(RenderTexture* texture, Camera cam) {
	IntRect rect; // Везде нужно округление, иначе текстура жестко дрожит
	rect.top = round(obj.get_sprite()->getGlobalBounds().top);
	rect.left = round(obj.get_sprite()->getGlobalBounds().left);
	rect.width = round(obj.get_sprite()->getGlobalBounds().width);
	rect.height = round(obj.get_sprite()->getGlobalBounds().height);

	s_effect_temp.setPosition(obj.get_sprite()->getPosition());
	s_effect_temp.setTexture(texture->getTexture());
	s_effect_temp.setTextureRect(rect);

	shader->setUniform("diffuse_texture", Shader::CurrentTexture);
	shader->setUniform("effect_texture", *obj.get_sprite()->getTexture());
	shader->setUniform("camera_size", Vector2f(cam.world_size));
	shader->setUniform("time", clock.getElapsedTime().asSeconds());

	shader->setUniform("effect_texture_pos", Vector2f(float(rect.left) / cam.resolution.x, 1.f - float(rect.top + rect.height) / cam.resolution.y));
	shader->setUniform("effect_texture_size", Vector2f(float(rect.width) / cam.resolution.x, float(rect.height) / cam.resolution.y));

	for (short i = 0; i < shader_attrs.size(); i++) { // Перекидываем все uniform в шейдеры
		shader->setUniform(shader_attrs[i].first, shader_attrs[i].second);
	}

	// Шейдер проходит по пикселям diffuse текстуры, на которые накладывается эффект, а спрайт эффекта передается как аргумент
	// Но вообще их размеры пропорциональны друг другу и для обоих можно использовать gl_TexCoord[0]
	effect_temp->draw(s_effect_temp, shader);
	effect_temp->display();
	s_effect_temp.setTexture(effect_temp->getTexture());

	texture->draw(s_effect_temp);
}

void gr::Light_source::var_update(Camera& cam) {
	DoubleRect rect = cam.world_rect;
	rect.top -= world_size.y;
	rect.left -= world_size.x;
	rect.width += 2 * world_size.x;
	rect.height += 2 * world_size.y;

	if (rect.contains(world_pos))
		visible = true;
	else {
		visible = false; // ПРОТЕСТИРОВАТЬ ПРОВЕРКУ
		return;
	}

	screen_pos.x = (world_pos.x - cam.corner_pos.x) / cam.world_size.x;
	screen_pos.y = 1.0 - (cam.corner_pos.y - world_pos.y) / cam.world_size.y;

	screen_size.x = world_size.x / (2.0 * cam.world_size.x);
	screen_size.y = world_size.y / (2.0 * cam.world_size.y);
}

Vector3f gr::Light_source::animation_update() {
	if (!active) {
		current_color = Vector3f(0.f, 0.f, 0.f);
	}
	if (colors.size() == 1) {
		current_color = colors[0].first;
		return current_color;
	}
	Time time = animation_clock.getElapsedTime() % full_animation_time;

	unsigned short curr = 0; // Ищем текущий кадр анимации цвета
	for (unsigned short i = 0; i < colors.size(); i++) { // Здесь можно поставить бин поиск для небольшого выигрыша в производительности
		if (time < colors[i].second) {
			curr = i;
			break;
		}
	}

	if (!smooth_brightness_change)
		current_color = colors[curr].first;
	else { // Если цвет меняется плавно, то считаем текущий свет
		if (curr > 0) {
			unsigned short prev = curr - 1;
			current_color = colors[prev].first + (colors[curr].first - colors[prev].first) * ((time - colors[prev].second) / (colors[curr].second - colors[prev].second));
		}
		else {
			Vector3f prev_color = colors[colors.size() - 1].first;
			current_color = prev_color + (colors[0].first - prev_color) * (time / colors[1].second);
		}

	}
	return current_color;

}

void gr::Light_source::load_from_file(ifstream& file) {
	file >> smooth_brightness_change;
	file >> world_pos.x >> world_pos.y >> world_size.x >> world_size.y >> direction.x >> direction.y;
	unsigned short anim_lenth;
	file >> anim_lenth;
	for (unsigned short i = 0; i < anim_lenth; i++) {
		Vector3f color;
		int col_time;
		file >> color.x >> color.y >> color.z >> col_time;
		full_animation_time += milliseconds(col_time);
		colors.push_back(pair<Vector3f, Time>(color, full_animation_time));
	}

}

void gr::Light_source::print_info() {
	cout << " === Light Source Info === " << endl;
	cout << "World position: " << world_pos << "  World size: " << world_size << "  Direction: " << direction << endl;
	cout << "Is visible now: " << visible << "  Current color: " << current_color << endl;
	cout << "Screen pos now: " << screen_pos << "  and screen size: " << screen_size << endl;
	cout << "Animation consists of " << colors.size() << " colors, total animation time: " << full_animation_time.asMilliseconds() << endl;
}

void gr::Graphics_engine::delete_sprite_object(vector<gr::Sprite_object*>::iterator index) { // Функция для быстрого удаления указателя из вектора спрайтов-обьектов
	if (index >= sprite_objects.end() || index < sprite_objects.begin()) {
		console->log("Trying to delete a non-existent pointer on sprite-object in graphics engine", ConsoleMessageType::WARN);
		return;
	}
	(*index) = (*(sprite_objects.end() - 1)); // быстро меняем удаляемый указатель на указатель из конца списка
	sprite_objects.pop_back(); // а теперь тоже
}

gr::Graphics_engine::Graphics_engine(RenderWindow& _window, ifstream& file) {
	Clock init_clock;
	window = &_window;

	if (!sogl_loadOpenGL()) {
		const char** failures = sogl_getFailures();
		console->log("Error loading", ConsoleMessageType::ERR);
	}
	else {
		console->log("Sogl load succsessfull!", ConsoleMessageType::SUCC);
	}
	glTextureBarrier = sogl_loadOpenGLFunction("glTextureBarrier");
	sogl_cleanup();

	//file >> render_resolution.x >> render_resolution.y >> output_resolution.x >> output_resolution.y; // Грузим разрешение рендера и отрисовки
	//main_camera.resolution = render_resolution;

	// !!!!! FULLSCREEN DEFAULT !!!!!
	render_resolution = VideoMode::getDesktopMode().size;
	output_resolution = render_resolution;
	main_camera.resolution = render_resolution;

	// Выделяем память для массивов, которые будут отправлять данные об освещении в шейдеры
	light_positions = new Glsl::Vec4[MAX_LIGHT_COUNT];
	light_directions = new Glsl::Vec2[MAX_LIGHT_COUNT];
	light_colors = new Glsl::Vec3[MAX_LIGHT_COUNT];

	// Загружаем шейдеры
	if (light_map_shader.loadFromFile("resources/shaders/light_map_shader.txt", Shader::Fragment) &&
		light_render_shader.loadFromFile("resources/shaders/light_render_shader.txt", Shader::Fragment) &&
		combine_shader.loadFromFile("resources/shaders/combine_shader.txt", Shader::Fragment) &&
		posteffect_shader.loadFromFile("resources/shaders/posteffect_shader.txt", Shader::Fragment) &&
		height_shader.loadFromFile("resources/shaders/height_shader.txt", Shader::Fragment) &&
		weather_shader.loadFromFile("resources/shaders/weather_shader.txt", Shader::Fragment))
	{
		console->log("Shader load successfull!", ConsoleMessageType::SUCC);
	}
	else {
		console->log("Error while loading shaders!", ConsoleMessageType::ERR);

	}

	// Создаем рендер-текстуры нужного размера 
	if (diffuse_result.create(Vector2u(render_resolution.x, render_resolution.y)) &&
		height_result.create(Vector2u(render_resolution.x, render_resolution.y)) &&
		light_result.create(Vector2u(render_resolution.x, render_resolution.y)) &&
		combined_result.create(Vector2u(render_resolution.x, render_resolution.y)) &&
		posteffect_result.create(Vector2u(render_resolution.x, render_resolution.y)) &&
		weather_result.create(Vector2u(render_resolution.x, render_resolution.y)) &&
		effect_temp.create(Vector2u(render_resolution.x, render_resolution.y)) &&
		light_map.create(Vector2u(768, 256)))
	{
		console->log("Render textures creating successful!", ConsoleMessageType::SUCC);
	}
	else {
		console->log("Error while creating render textures!", ConsoleMessageType::ERR);
	}

	combine_shader.setUniform("diffuse_result", weather_result.getTexture()); // Устанавливем кучу параметров в шейдеры
	combine_shader.setUniform("light_result", light_result.getTexture());
	posteffect_shader.setUniform("texture", Shader::CurrentTexture);
	posteffect_shader.setUniform("texture_size", Vector2f(render_resolution));
	combine_shader.setUniform("diffuse_result", weather_result.getTexture());
	combine_shader.setUniform("light_result", light_result.getTexture());
	output_sprite.setTexture(posteffect_result.getTexture());
	output_sprite.setTextureRect(IntRect(Vector2i(render_resolution - output_resolution) / 2, Vector2i(output_resolution)));
	height_shader.setUniform("sprite", Shader::CurrentTexture);
	height_shader.setUniform("height_result", height_result.getTexture());
	height_shader.setUniform("height_resolution", Vector2f(render_resolution));
	weather_shader.setUniform("diffuse_result", diffuse_result.getTexture());
	weather_shader.setUniform("height_result", height_result.getTexture());

	fps_text.setFont(console->font);
	fps_text.setCharacterSize(16);

	console->log("Graphics engine configured in " + to_string(init_clock.getElapsedTime().asMilliseconds()) + " milliseconds", ConsoleMessageType::INFO);
}

void gr::Graphics_engine::create_draw_order() {
	// Сначала очищаем очередь
	sprite_draw_order.clear();

	// Ищем спрайты, попадающие в область видимости и добавляем в очередь
	for (int obj_num = 0; obj_num < sprite_objects.size(); obj_num++) {

		// Код для удаления спрайта
		bool obj_deleted = false;
		for (int i = 0; i < int(sprite_objects_for_deletion.size()); i++) { // проверяем некоторое количество элементов из списка для удаления
			if (sprite_objects[obj_num] == sprite_objects_for_deletion[i]) { // если отрисовываемый обьект 
				delete_sprite_object(sprite_objects.begin() + obj_num);
				sprite_objects_for_deletion.erase(sprite_objects_for_deletion.begin() + i); // тут можно просто стереть т.к. список не предполагается особо большим
				obj_deleted = true;
				break;
			}
		}
		if (obj_deleted) {
			obj_num--;
			continue;
		}

		if (sprite_objects[obj_num]->var_update(main_camera) && sprite_objects[obj_num]->active) {
			sprite_draw_order.push_back(sprite_objects[obj_num]);
		}
	}
	// Сортируем их по слою отрисовки layer
	sort(sprite_draw_order.begin(), sprite_draw_order.end(),
		[](Sprite_object* obj1, Sprite_object* obj2) {return (obj1->get_layer() > obj2->get_layer()); });

	// То-же для эффектов
	effect_draw_order.clear();
	for (auto obj = effects.begin(); obj < effects.end(); obj++) {
		if ((*obj)->obj.var_update(main_camera)) {
			effect_draw_order.push_back(*obj);
		}
	}
	sort(effect_draw_order.begin(), effect_draw_order.end(),
		[](Effect* obj1, Effect* obj2) {return (obj1->obj.get_layer() > obj2->obj.get_layer()); });

	// Аналогичные действия делаем для спрайтов с картами высот, но уже можно не сортировать
	height_draw_order.clear();
	for (int obj_num = 0; obj_num < height_objects.size(); obj_num++) {
		if (height_objects[obj_num]->var_update(main_camera) && height_objects[obj_num]->active) {
			height_draw_order.push_back(height_objects[obj_num]);
		}
	}
}

void gr::Graphics_engine::update_lights() {
	// Обновляем массивы с информацией об источниках
	int light_counter = 0;
	for (auto obj = lights.begin(); obj < lights.end(); obj++) {
		(*obj)->var_update(main_camera);
		if ((*obj)->visible && light_counter < MAX_LIGHT_COUNT && (*obj)->active) {
			(*obj)->animation_update();
			light_positions[light_counter].x = (*obj)->screen_pos.x;
			light_positions[light_counter].y = (*obj)->screen_pos.y;
			light_positions[light_counter].z = (*obj)->screen_size.x;
			light_positions[light_counter].w = (*obj)->screen_size.y;

			light_directions[light_counter].x = (*obj)->direction.x;
			light_directions[light_counter].y = (*obj)->direction.y;

			light_colors[light_counter].x = (*obj)->current_color.x;
			light_colors[light_counter].y = (*obj)->current_color.y;
			light_colors[light_counter].z = (*obj)->current_color.z;

			light_counter += 1;
		}
	}
	// И закидываем все в шейдеры
	light_map_shader.setUniform("light_count", light_counter);
	light_map_shader.setUniformArray("light_positions", light_positions, light_counter);
	light_map_shader.setUniformArray("light_directions", light_directions, light_counter);
	light_map_shader.setUniform("obstacle_map", height_result.getTexture());
	light_map_shader.setUniform("world_camera_size", Vector2f(main_camera.world_size));
	light_map_shader.setUniform("light_map_resolution", Vector2f(light_map.getSize()));

	light_render_shader.setUniform("light_count", light_counter);
	light_render_shader.setUniformArray("light_positions", light_positions, light_counter);
	light_render_shader.setUniformArray("light_colors", light_colors, light_counter);
	light_render_shader.setUniform("height_map", height_result.getTexture());
	light_render_shader.setUniform("def_color", ambient_light);
	light_render_shader.setUniform("world_camera_size", Vector2f(main_camera.world_size));
	light_render_shader.setUniform("light_map_resolution", Vector2f(light_map.getSize()));
	light_render_shader.setUniform("light_map", light_map.getTexture());
}

void gr::Graphics_engine::set_weather_uniform() {
	shader_wind_dist += (clock.getElapsedTime() - wind_last_time).asSeconds() * Vector2f(wind_speed.x, -wind_speed.y);
	weather_shader.setUniform("world_pos", Vector2f(main_camera.world_pos));
	weather_shader.setUniform("world_size", Vector2f(main_camera.world_size));
	weather_shader.setUniform("wind_speed", Vector2f(wind_speed.x, -wind_speed.y)); // Минус для перевода в координаты glsl
	weather_shader.setUniform("fog_height", fog_height); // Высота тумана
	weather_shader.setUniform("fog_def", fog_def); // Плотность тумана
	weather_shader.setUniform("fog_cloud", fog_cloud);
	weather_shader.setUniform("rain_strength", rain * 0.0023f); // Интенсивность дождя на подгонный коэффициент
	weather_shader.setUniform("wind_dist", shader_wind_dist);
	wind_last_time = clock.getElapsedTime();
}

void gr::Graphics_engine::set_posteffect_uniform() {
	posteffect_shader.setUniform("corner_shading_radius", corner_shading_radius);
	posteffect_shader.setUniform("corner_shading_color", corner_shading_color);
	posteffect_shader.setUniform("color_correction_r", color_correction_r);
	posteffect_shader.setUniform("color_correction_g", color_correction_g);
	posteffect_shader.setUniform("color_correction_b", color_correction_b);
	posteffect_shader.setUniform("noise_alpha", noise_alpha);
	posteffect_shader.setUniform("noise_fps", noise_fps);
	posteffect_shader.setUniform("noise_size", noise_size);
	posteffect_shader.setUniform("aberration_radius", aberration_radius);
	posteffect_shader.setUniform("aberration_shift", aberration_shift);
	posteffect_shader.setUniform("gamma", gamma);
}

void gr::Graphics_engine::render() {
	/*
	diffuse_result --(adding weather)--> weather_result -\
														  |-(combining diffuse with weather and light)--> combined_result --> posteffect_result
	height_result ----> light_map ----> light_result ----/
	*/

	// Очищаем текстуры (можно чистить не все)
	diffuse_result.clear(null_color);
	height_result.clear(null_color);

	// Обновляем камеру и создаем очередь на отрисовку
	main_camera.update();
	create_draw_order();

	// Отрисовывем спрайты по порядку в обычную diffuse текстуру и карту высот
	auto eff = effect_draw_order.begin();
	for (auto obj = sprite_draw_order.begin(); obj < sprite_draw_order.end(); obj++) {

		while (eff != effect_draw_order.end()) {
			if ((*eff)->obj.get_layer() > (*obj)->get_layer()) {
				(*eff)->draw(&diffuse_result, main_camera);
				eff++;
			}
			else {
				break;
			}
		}
		//cout << (*obj)->get_texture_name() << endl;
		diffuse_result.draw(*(*obj)->get_sprite());
	}
	while (eff != effect_draw_order.end()) {
		(*eff)->draw(&diffuse_result, main_camera);
		eff++;
	}


	for (auto obj = height_draw_order.begin(); obj < height_draw_order.end(); obj++) {
		height_result.draw(*(*obj)->get_sprite(), &height_shader);
		((void (*)(void)) glTextureBarrier)();
	}
	diffuse_result.display();
	height_result.display();
	// На данном этапе у нас готовы две главные текстуры которые далее с помощью шейдеров будут приводится к нормальному виду

	update_lights();

	// Создаем карту освещения
	light_map.draw(Sprite(light_map.getTexture()), &light_map_shader);

	// Рисуем все тени в отдельную текстуру
	light_result.draw(Sprite(light_result.getTexture()), &light_render_shader);
	light_result.display();

	// Обновляем информацию в шейдере погоды и постэффектов, это можно делать не каждый кадр, а чуть реже ради производительностиэ
	if (frame_counter % shader_update_freq == 0) {
		set_posteffect_uniform();
	}
	set_weather_uniform();

	// На diffuse_result добавляем погоду и получаем weather_result
	weather_shader.setUniform("u_time", clock.getElapsedTime().asSeconds());
	weather_result.draw(Sprite(diffuse_result.getTexture()), &weather_shader);

	// Закидываем weather_result в шейдре и складываем обычную цветастую картинку с тенями вместе
	combined_result.draw(Sprite(weather_result.getTexture()), &combine_shader);

	// Добавляем постэффекты к картинке
	posteffect_shader.setUniform("time", clock.getElapsedTime().asSeconds());
	posteffect_result.draw(Sprite(combined_result.getTexture()), &posteffect_shader);
	posteffect_result.display();

	// Выводим итоговый спрайт на окно
	window->draw(output_sprite);
	frame_counter++;

	// Анализ производительности (он сам жрет производительности)
	if (frame_time.size() == frame_num_for_anylysis) {
		sort(frame_time.begin(), frame_time.end());
		fps = float(frame_num_for_anylysis) / sum_time;
		sum_time = 0.f;

		float one_percent_low_time = 0.f, ten_percent_low_time = 0.f;
		for (int i = 0; i < frame_num_for_anylysis / 10; i++) {
			ten_percent_low_time += frame_time[frame_num_for_anylysis - i - 1];
			if (i < frame_num_for_anylysis / 100) {
				one_percent_low_time += frame_time[frame_num_for_anylysis - i - 1];
			}
		}
		one_percent_low = float(frame_num_for_anylysis / 100) / one_percent_low_time;
		ten_percent_low = float(frame_num_for_anylysis / 10) / ten_percent_low_time;
		frame_time.clear();
		fps_text.setString("FPS: " + to_string(fps).erase(5) + "  10% low: " + to_string(ten_percent_low).erase(5) + "  1% low: " + to_string(one_percent_low).erase(5));
	}
	window->draw(fps_text);
	float last_frame_time = fps_clock.getElapsedTime().asSeconds();
	fps_clock.restart();
	frame_time.push_back(last_frame_time);
	sum_time += last_frame_time;

	color_correction_r = Glsl::Vec3(1.f, 0.f, 0.f);
	color_correction_g = Glsl::Vec3(0.f, 1.f, 0.f);
	color_correction_b = Glsl::Vec3(0.f, 0.f, 1.f);
}

void gr::Graphics_engine::load_from_file(ifstream& file) {
	file >> main_camera.world_pos.x >> main_camera.world_pos.y >> main_camera.world_size.x >> main_camera.world_size.y;
	int load_counter = 0;
	Clock load_clock;

	char type;
	while (file >> type) {
		if (type == 's') {
			Sprite_object* obj = new Sprite_object;
			obj->load_from_file(file);
			sprite_objects.push_back(obj);
		}
		if (type == 'h') {
			Sprite_object* obj = new Sprite_object;
			obj->load_from_file(file);
			height_objects.push_back(obj);
		}
		if (type == 'l') {
			Light_source* src = new Light_source;
			src->load_from_file(file);
			lights.push_back(src);
		}
		if (type == 'e') {
			Effect* eff = new Effect;
			eff->load_from_file(file);
			eff->effect_temp = &effect_temp;
			effects.push_back(eff);
		}
		load_counter++;
	}
	console->log("Loaded " + to_string(load_counter) + " graphics objects in " + to_string(load_clock.getElapsedTime().asMilliseconds()) + " milliseconds", ConsoleMessageType::INFO);
}

void gr::Graphics_engine::print_info() {
	cout << " === Graphics Engine Info === " << endl;
	cout << " Render resolution: " << main_camera.resolution << endl;
	cout << " Main camera position: " << main_camera.world_pos << "  and it`s world size: " << main_camera.world_size << endl;

	cout << " Sprite objects info: " << endl;
	for (auto obj = sprite_objects.begin(); obj < sprite_objects.end(); obj++) {
		(*obj)->print_info();
	}

	cout << " Height objects info: " << endl;
	for (auto obj = height_objects.begin(); obj < height_objects.end(); obj++) {
		(*obj)->print_info();
	}

	cout << " Light sources info: " << endl;
	for (auto obj = lights.begin(); obj < lights.end(); obj++) {
		(*obj)->print_info();
	}

	cout << " === End of Graphics Engine Info === " << endl;
}

gr::Graphics_engine::~Graphics_engine() {
	for (auto obj = sprite_objects.begin(); obj < sprite_objects.end(); obj++) {
		delete* obj;
	}

	for (auto obj = height_objects.begin(); obj < height_objects.end(); obj++) {
		delete* obj;
	}

	for (auto obj = lights.begin(); obj < lights.end(); obj++) {
		delete* obj;
	}
}
