#pragma once

#include "Graphics.h"
#include "Common.h"

using namespace sf;
using namespace std;
using namespace gr;

Glsl::Vec3 operator*(Glsl::Vec3 vec, float f);

Glsl::Vec4 operator*(Glsl::Vec4 vec, float f);

Glsl::Vec4 operator+(Glsl::Vec4 vec1, Glsl::Vec4 vec2);

Glsl::Vec3 operator+(Glsl::Vec3 vec1, Glsl::Vec3 vec2);

class Light_cycle {
public:
	vector<pair<double, Vector3f>> light_color; // Пары из <время от начала суток в часа, фоновое освещение>

	// Может стоит добавить другую формулу для shift_koeff, как например smoothstep в glsl
	Vector3f get_light_color(double time);

	void load_from_file(ifstream& file);
};

// Класс состояния погоды
class Weather_state {
public:
	string weather_name; // Название погоды

	int probability; // Вероятность погоды (потом сумма вероятностей всех погод нормируется на 1)
	float avg_time; // Средняя продолжительность погоды (в часах игрового времени)

	Light_cycle cycle;

	float wind_mod = 0.f; // Скорость ветра (без направления)
	float fog_def = 0.f; // Сила тумана по умолчанию
	float fog_cloud = 0.f; // Сила дополнительных облачков тумана
	float fog_height = 0.f; // Высота тумана
	float rain = 0.f; // Интенсивность дождя порядка ~[0.0, 1.5]
	float leaves = 0.f; // Колчиество летящих листиков в ветре
	float thunder = 0.f; // Колчисетво (вероятность появления) раскатов грома

	float corner_shading_radius = 0.f;
	Glsl::Vec4 corner_shading_color = Glsl::Vec4(0.f, 0.f, 0.f, 0.f);

	Glsl::Vec3 color_correction_r = Glsl::Vec3(1.f, 0.f, 0.f);
	Glsl::Vec3 color_correction_g = Glsl::Vec3(0.f, 1.f, 0.f);
	Glsl::Vec3 color_correction_b = Glsl::Vec3(0.f, 0.f, 1.f);
	float noise_alpha = 0.f;

	void load_from_file(ifstream& file);
};

// Класс всей системы погоды
class Weather_system {
private:
	const double TWOPI = 6.28318530718;

	Graphics_engine* eng; // Указатель на графический движок
	Game_time* game_time; // Игровое время
	Console* console = Console::get_instance();

	int total_probability = 0;

	deque<pair<Weather_state*, float>> weather_order; // Последовательность погоды вместе со временем
	double last_state_time = 0.f;

	float wind_angle = 0.f; // [0 .. 2*PI]
	float current_wind_mod = 0.f;
	int last_update_second = 0;

	vector<Sprite_object> leaf_samples; // Образцы для листиков, которые будем копировать
	vector<pair<Sprite_object*, Vector2d>> leaves; // Копии листиков, которые как раз двигаются по экрану и их скорости(они чуток отличаются от скорости ветра для рандома)

	const float leaves_max_density = 0.4f; // Количество листиков на еденицу площади камеры при значении 1.0 соответсвующего параметра в погоде
	const int max_leaves = 300;
	const float min_wind_mod = 0.5f; // Минимальная скорость ветра чтобы было можно спавнить листики (при маленькой скорости листики будут выглядеть странно)
	const double leaves_delete_koeff = 1.5f; // Коэффициент для расстояния от листика до камеры на котором будут удалятся листики
	const double leaves_spawn_koeff = 1.05f; // Коэффициент для расстояния от листика до экрана на котором будут спавнятся листики
	const float leaves_angle_delta = 0.12f; // Какой угол отклоенения от направления ветра может иметь направление движения листика
	Clock leaf_clock;
	Clock leaf_spawn_clock;
	const Time leaf_spawn_cooldown = seconds(10.0);

	// Придумать как закидывать листики в движок для отрисовки, чтобы их не удалять постоянно и подкорректировать алгоритм
	void update_leaves();

public:
	double weather_shift_koeff = 0.0;
	vector<Weather_state> states; // Все состояния погоды

	Weather_system(Graphics_engine* _eng, Game_time* _time);

	// Прочитать из файла все состояния погоды
	void load_from_file(ifstream& file);

	// Добавить в очередь новые погодные состояния в количестве num_of_states штук
	void create_order(int num_of_states);

	void update();
};