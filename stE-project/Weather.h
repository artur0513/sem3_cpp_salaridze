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
	vector<pair<double, Vector3f>> light_color; // ���� �� <����� �� ������ ����� � ����, ������� ���������>

	// ����� ����� �������� ������ ������� ��� shift_koeff, ��� �������� smoothstep � glsl
	Vector3f get_light_color(double time);

	void load_from_file(ifstream& file);
};

// ����� ��������� ������
class Weather_state {
public:
	string weather_name; // �������� ������

	int probability; // ����������� ������ (����� ����� ������������ ���� ����� ����������� �� 1)
	float avg_time; // ������� ����������������� ������ (� ����� �������� �������)

	Light_cycle cycle;

	float wind_mod = 0.f; // �������� ����� (��� �����������)
	float fog_def = 0.f; // ���� ������ �� ���������
	float fog_cloud = 0.f; // ���� �������������� �������� ������
	float fog_height = 0.f; // ������ ������
	float rain = 0.f; // ������������� ����� ������� ~[0.0, 1.5]
	float leaves = 0.f; // ���������� ������� �������� � �����
	float thunder = 0.f; // ���������� (����������� ���������) �������� �����

	float corner_shading_radius = 0.f;
	Glsl::Vec4 corner_shading_color = Glsl::Vec4(0.f, 0.f, 0.f, 0.f);

	Glsl::Vec3 color_correction_r = Glsl::Vec3(1.f, 0.f, 0.f);
	Glsl::Vec3 color_correction_g = Glsl::Vec3(0.f, 1.f, 0.f);
	Glsl::Vec3 color_correction_b = Glsl::Vec3(0.f, 0.f, 1.f);
	float noise_alpha = 0.f;

	void load_from_file(ifstream& file);
};

// ����� ���� ������� ������
class Weather_system {
private:
	const double TWOPI = 6.28318530718;

	Graphics_engine* eng; // ��������� �� ����������� ������
	Game_time* game_time; // ������� �����
	Console* console = Console::get_instance();

	int total_probability = 0;

	deque<pair<Weather_state*, float>> weather_order; // ������������������ ������ ������ �� ��������
	double last_state_time = 0.f;

	float wind_angle = 0.f; // [0 .. 2*PI]
	float current_wind_mod = 0.f;
	int last_update_second = 0;

	vector<Sprite_object> leaf_samples; // ������� ��� ��������, ������� ����� ����������
	vector<pair<Sprite_object*, Vector2d>> leaves; // ����� ��������, ������� ��� ��� ��������� �� ������ � �� ��������(��� ����� ���������� �� �������� ����� ��� �������)

	const float leaves_max_density = 0.4f; // ���������� �������� �� ������� ������� ������ ��� �������� 1.0 ��������������� ��������� � ������
	const int max_leaves = 300;
	const float min_wind_mod = 0.5f; // ����������� �������� ����� ����� ���� ����� �������� ������� (��� ��������� �������� ������� ����� ��������� �������)
	const double leaves_delete_koeff = 1.5f; // ����������� ��� ���������� �� ������� �� ������ �� ������� ����� �������� �������
	const double leaves_spawn_koeff = 1.05f; // ����������� ��� ���������� �� ������� �� ������ �� ������� ����� ��������� �������
	const float leaves_angle_delta = 0.12f; // ����� ���� ����������� �� ����������� ����� ����� ����� ����������� �������� �������
	Clock leaf_clock;
	Clock leaf_spawn_clock;
	const Time leaf_spawn_cooldown = seconds(10.0);

	// ��������� ��� ���������� ������� � ������ ��� ���������, ����� �� �� ������� ��������� � ����������������� ��������
	void update_leaves();

public:
	double weather_shift_koeff = 0.0;
	vector<Weather_state> states; // ��� ��������� ������

	Weather_system(Graphics_engine* _eng, Game_time* _time);

	// ��������� �� ����� ��� ��������� ������
	void load_from_file(ifstream& file);

	// �������� � ������� ����� �������� ��������� � ���������� num_of_states ����
	void create_order(int num_of_states);

	void update();
};