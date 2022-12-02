#pragma once
#define SOGL_MAJOR_VERSION 4
#define SOGL_MINOR_VERSION 5
#define SOGL_ARB_texture_barrier

#ifdef __linux__ 
#define SOGL_IMPLEMENTATION_X11
#elif _WIN32
#define SOGL_IMPLEMENTATION_WIN32
#endif

#include "Common.h"
#include "Console.h"
#include <filesystem>

using namespace std::filesystem;

// =================================
// ����� ��� ����������� ����� (���� ����) ���� �������� �������� �����, � �� ����� ������ �������
// �� ����� ����� ���� �� �������� ������������ ����� � ��������, �������� ��������� ������ �������� ���� std::variant ��� ���� ����� ����������� �������� ��� �������������,
// �� ��� ������� ������, �������� ���� ������� � �������� ����� ��������.
// ������� �� ������ ������ ��� ����, ����� ����� - ��������.
// =================================

namespace gr {

	class Camera {
	private:
		Vector2d camera_speed; // ��� �������� ������ ����������� �� ��������� �� ���������, � �� ��������
		Vector2d prev_pos = Vector2d(0, 0);
		Clock clock;
		Console* console = Console::get_instance();
		unsigned long tick_count = 1;

	public:
		Vector2d world_pos, world_size; // ���������� �������� ������; � ������� �������, ������������ �������
		Vector2u resolution; // ������ �������, ������� ����� ��������� � �������� (���������� ������)

		Vector2d corner_pos; // ���������� ������ �������� ���� ������
		DoubleRect world_rect; // ������������� �������, ������������ �������
		float aspect_ratio; // ����������� ������

		void update();

		// ������� ��� �������� ������� ��������� � ���������� ������ �������� (��� ���������� ������, ����������� �� �������, � �� ����� ����)
		Vector2f world_to_camera_pos(Vector2d gl_pos);

		// ������� ��� �������� ��������� ������ �������� � ������� (������������ Ũ �������, � �� ����� ����)
		Vector2d camera_to_world_pos(Vector2f c_pos);

		Vector2f world_to_glsl_pos(Vector2d gl_pos);

		Vector2d get_speed();
	};


	// �����-�������� ��� �������� ������� � ��������, ��������� �������������� ������� �������� ���������� �������/�������� � ������ ��� ��������
	template<class T>
	class Resource_manager {
	private:
		map<string, T*> resources;

		Resource_manager() {};
		Resource_manager(const Resource_manager& r) {};
		Resource_manager operator=(const Resource_manager& r) {};
		Resource_manager(Resource_manager&& r) {};
		Resource_manager& operator=(const Resource_manager&& r) {};

		bool load_object_from_file(T* obj, string filename); // ����� �������������� �������������� ��� ������� �������� ��� ��� T
	public:
		static Resource_manager* get_instance() {
			static Resource_manager instance;
			return &instance;
		}

		void preload(string filename) {
			ifstream file(filename.c_str());
			if (file.is_open()) {
				std::string object_filename;
				while (getline(file, object_filename)) { // ��������� �������� ����� �������
					if (object_filename.size() == 0) // ���� ������ ������ 0
						continue;
					else if (object_filename[object_filename.find_first_not_of(" ")] == '#') // ��� ��� ���������� � ������� ��� �����������
						continue; // �� ��������� � ��������� ������

					object_filename = relative(object_filename).string(); // ������� �������� ����� � ������� ����
					T* new_object = new T; // ������� ��������� �� ������

					if (!load_object_from_file(new_object, object_filename)) // ������� ������� �� �����
						delete new_object; // ���� �� �������, �� ������� � ���� ������
					else {
						resources.insert(pair<string, T*>(object_filename, new_object)); // ����� ���������� � ����� � ���� ���� ������
					}
				}
			}
			else {
				cout << "cannot load objects, file '" << filename << "' not found!" << endl;
			}
			file.close();
		}

		T* get_object(string filename) {
			filename = relative(filename).string(); // ������� �������� ����� � ������� ����
			auto itrtr = resources.find(filename);

			if (itrtr == resources.end()) { // ���� ������� ������� ����, �� ��������� ��� �����, �� ������ ����� ����� ������� �������
				T* new_object = new T; // ������� ��������� �� ������

				if (!load_object_from_file(new_object, filename)) { // ������� ������� �� �����
					return nullptr;
				}
				else {
					resources.insert(pair<string, T*>(filename, new_object)); // ����� ���������� � �����
					return new_object; // � ���������� ��������� �� ����� ������
				}
			}
			else // ���� ������ ��� �� �����, �� ���������� �� ���� ���������
				return itrtr->second;
		}

		~Resource_manager() {
			auto itrtr = resources.begin();
			while (itrtr != resources.end()) {
				delete itrtr->second;
				itrtr++;
			}
		}
	};


	// ����� ��� �������� �������� � ���� ������������������ �� ��� <������ - �����, ������� ����� ��� ����������>
	class Animation {
	private:
		vector<pair<Sprite, Time>> frames; // ������ � ���� ������� � ����� �� ������ �������� �� ������ ���������������� �������

	public:
		Time full_animation_time = milliseconds(0);
		unsigned short number_of_frames = 0;

		float speed_koeff = 1.f; // ����������� �������� �������������� �������

		void add_frame(Sprite _sprite, Time _time);

		Sprite* get_frame(Time _time);

		Time get_frame_time(unsigned short i);

		IntRect get_texture_rect(unsigned short i);

	};

	// ������ ����� ��� �������� � ������� ����
	class Sprite_object {
	private:
		Sprite def_sprite;// ������ �� ���������, ����� �� ������������� ������� �������� � ������ � ������ �����
		Sprite* current_sprite;// ��������� �� ������� ���� �������� (��� ���� �� ���������)

		double delta_layer, layer; // �������� ���������� ��� ������� ��������� ������������ ������ ������� ������� � ���� ���������� ���������
		string texture_name, asset_name; // �������������� ��������� ��� �������� ���������� � ������������ �������� � ������(��� ���������)

		Clock animation_clock; // ������ ��� ������������ ������� ��������

		bool visible; // ��������� �������
	public:
		vector<Animation> animations; // ������ � ���������� �������
		Vector2d world_pos, world_size; // �������(����� ������� ����) � ������ � ������� ����
		unsigned short id; // ���� �������������� ���������� ��� �������� id �������
		Vector2u pixel_local_size; // ��������� ������ ������� � ��������, ��� ����� ��������� ��������
		short current_animation = -1; // ������������� �� ������ ������ ��������, -1 ���� ������ �� ���������
		bool active = true;

		// ���������� ���������, ��������� � ������������ � ��������� �������, ���������� � ����� ���������
		bool var_update(Camera& cam);

		void animation_update();

		// ��������� ������ �� �����
		void load_from_file(ifstream& file);

		// ���������� ������� � ����
		void save_to_file(ofstream& file);

		// ������� �������� ���������� � ������� � �������
		void print_info();

		Sprite* get_sprite();

		double get_layer();

		void set_delta_layer(double _delta_layer);

		string get_texture_name();
	};

	// ����� ��� ������������� ��������
	class Effect {
	private:
		Shader* shader; // ��������� �� ������ ������� �������

		vector<pair<string, float>> shader_attrs; // �������� ������� (����� ���� ������ float)

		string shader_name;
		Clock clock;
	public:
		Sprite_object obj; // ������ �� ������ �������� ����� ����������� ������ (�������� � ��� ����� ���� distortion texture)
		RenderTexture* effect_temp; // ��������� �� ������������� �������� ��� ������� �������� (��� ���� ����� �� ��� �������) ����� ������� ��� ��������
		Sprite s_effect_temp;

		void load_from_file(ifstream& file);

		// �������� ������� ������� �� ������ �������� �� ������� ������ ����� �������������
		void draw(RenderTexture* texture, Camera cam);
	};

	// ����� ��� �������� ���������� �� ���������� �����
	class Light_source {
	private:
		vector<pair<Vector3f, Time>> colors; // ����������� ����� ��������� (� �������������� �������) �� �������
		bool smooth_brightness_change = false; // ������� ��������� ������� ��� ��������

		Time full_animation_time = milliseconds(0);
		Clock animation_clock;
	public:
		bool active = true; // ������� ��� �������� �������� ����� ������
		Vector2d world_pos; // ������� ��������� ����� � ������� ���� (��������)
		Vector2d world_size; // ������ �������, ������� ���������� ����
		bool visible = false; // ������� �� ���� �� ��������� �� ������
		Vector2f screen_pos, screen_size; // ��� ���� �������� ������������ �� ������� ������ ��� �������
		Vector2f direction; // ����������� �����(direction.x) � ���� � ������� ������ ��������(direction.y)
		Vector3f current_color;

		void var_update(Camera& cam);

		Vector3f animation_update();

		void load_from_file(ifstream& file);

		void print_info();
	};

	// ����������� ������, ������������ � ���� ��� ��������� ������
	class Graphics_engine {
	private:
		const Color null_color = Color(0, 0, 0, 0);
		const int MAX_LIGHT_COUNT = 32;
		const int shader_update_freq = 20;
		Console* console = Console::get_instance();

		Clock clock; // �������� ���� ������������ ������
		long frame_counter = 0; // ������� ������

		// ������� ��������� ��� ���� ��������, ��������
		deque<Sprite_object*> sprite_draw_order;
		deque<Effect*> effect_draw_order;
		deque<Sprite_object*> height_draw_order;

		Vector2u render_resolution, output_resolution; // ���������� ������� � �������� ��������, ��� ������������� �� �����

		Shader light_map_shader; // ������ ��� ��������� ����� �������� ��� ���������� �����
		RenderTexture light_map; // ���� ����� ���������

		Shader light_render_shader; // ������ ��� ��������� ����� �� ����� ���������
		Glsl::Vec4* light_positions; // ������� � ����������� �� ���������� �����
		Glsl::Vec2* light_directions;
		Glsl::Vec3* light_colors;

		Shader combine_shader; // ������ ��� ���������� diffuse �������� � ������������ �����
		Shader posteffect_shader; // ������ � �������������
		Shader height_shader; // ������ ��� ����������� ��������� ���� �����(����� ��������� � �������������� TEXTURE_RECT � ��������)

		Shader weather_shader; // ������ ������
		Vector2f shader_wind_dist = Vector2f(0.f, 0.f); // ���������� ������ ����, ����� ��� ������� ������
		Time wind_last_time;

		RenderTexture diffuse_result, height_result, weather_result, light_result, combined_result, posteffect_result; // ������������� �������� ��� ���������
		Sprite output_sprite; // �������� ������ ���������� ��� ������ "������"

		RenderWindow* window;// ���� � ������� ����� ������������ �������� ������

		void(*glTextureBarrier);

		void delete_sprite_object(vector<Sprite_object*>::iterator index);

		Clock fps_clock; // ���� ��� ��������� ������������������
		vector<float> frame_time; // ������ ��� ������� �����
		int frame_num_for_anylysis = 600; // ���������� ������ ��� ������� ��� 
		float fps = 0.f, ten_percent_low = 0.f, one_percent_low = 0.f, sum_time = 0.f;
		Text fps_text;
	public:
		RenderTexture effect_temp; // ������������� �������� ��� ��������
		Camera main_camera; // �������� ������, �� ���� ������� ����� ���� ������

		// ������ � �����������
		Vector3f ambient_light = Vector3f(0.7, 0.7, 0.8);

		Vector2f wind_speed = Vector2f(0.f, 0.f); // �������� �����
		float fog_def = 0.f; // ���� ������ �� ���������
		float fog_cloud = 0.f; // ���� �������������� �������� ������
		float fog_height = 0.f; // ������ ������
		float rain = 0.f; // ������������� ����� ������� ~[0.0, 1.5]

		float corner_shading_radius = 0.f;
		Glsl::Vec4 corner_shading_color = Glsl::Vec4(0.f, 0.f, 0.f, 0.f);
		Glsl::Vec3 color_correction_r = Glsl::Vec3(1.f, 0.f, 0.f);
		Glsl::Vec3 color_correction_g = Glsl::Vec3(0.f, 1.f, 0.f);
		Glsl::Vec3 color_correction_b = Glsl::Vec3(0.f, 0.f, 1.f);
		float noise_alpha = 0.f;
		float noise_fps = 24.f;
		float noise_size = 3.f;
		float aberration_radius = 0.f;
		float aberration_shift = 0.f;
		float gamma = 1.0f;

		vector<Sprite_object*> sprite_objects; // ������� ������� � diffuse ���������
		vector<Sprite_object*> sprite_objects_for_deletion; // ������ �������� ��� �������� (��� ��������� �� �����, � ��� ���������, ����� �� ������� ������ ������)

		vector<Effect*> effects;// ������ ��������

		vector<Sprite_object*> height_objects;// ������� � ������ ����� ������ ��������

		vector<Light_source*> lights;// ������ � ����������� �����

		// � ������������ ����� ������� ����, � ������� �������� �������� ��������� � ���� � ����������� ��� ������
		Graphics_engine(RenderWindow& _window, ifstream& file);

		// ������� ������� ���������
		void create_draw_order();

		void update_lights();

		void set_weather_uniform();

		void set_posteffect_uniform();

		void render();

		void load_from_file(ifstream& file);

		void print_info();

		~Graphics_engine();
	};

}