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
// Почти вся графическая часть (этот файл) была написана довольно давно, и не очень хорошо сделана
// По факту можно было бы написать гооорааааздо лучше и приятнее, например используя всякие навороты типа std::variant для всех типов графических обьектов или наследованием,
// но это большая работа, особенно лень возится с огромной кучей шейдеров.
// Поэтому на данный момент как есть, будет время - перепишу.
// =================================

namespace gr {

	class Camera {
	private:
		Vector2d camera_speed; // Эта скорость камеры вычисляется на основании ее координат, а не наоборот
		Vector2d prev_pos = Vector2d(0, 0);
		Clock clock;
		Console* console = Console::get_instance();
		unsigned long tick_count = 1;

	public:
		Vector2d world_pos, world_size; // Координаты середины камеры; и размеры области, охватываемые камерой
		Vector2u resolution; // Размер области, которую нужно рендерить в пикселях (Разрешение экрана)

		Vector2d corner_pos; // Координаты левого верхнего угла камеры
		DoubleRect world_rect; // Прямоугольник области, охватываемой камерой
		float aspect_ratio; // Соотношение сторон

		void update();

		// Функция для перевода мировых координат в координаты рендер текстуры (для конкретной камеры, ОТНОИТЕЛЬНО ЕЕ ПОЗИЦИИ, А НЕ ВСЕГО ОКНА)
		Vector2f world_to_camera_pos(Vector2d gl_pos);

		// Функция для перевода координат рендер текстуры в мировые (ОТНОСИТЕЛЬНО ЕЁ ПОЗИЦИИ, А НЕ ВСЕГО ОКНА)
		Vector2d camera_to_world_pos(Vector2f c_pos);

		Vector2f world_to_glsl_pos(Vector2d gl_pos);

		Vector2d get_speed();
	};


	// Класс-менеджер для хранения текстур и шейдеров, стремится минимизировать попытки загрузки дубликатов текстур/шейдеров и просто для удобства
	template<class T>
	class Resource_manager {
	private:
		map<string, T*> resources;

		Resource_manager() {};
		Resource_manager(const Resource_manager& r) {};
		Resource_manager operator=(const Resource_manager& r) {};
		Resource_manager(Resource_manager&& r) {};
		Resource_manager& operator=(const Resource_manager&& r) {};

		bool load_object_from_file(T* obj, string filename); // Нужно самостоятельно переопределять эту функцию загрузки под тип T
	public:
		static Resource_manager* get_instance() {
			static Resource_manager instance;
			return &instance;
		}

		void preload(string filename) {
			ifstream file(filename.c_str());
			if (file.is_open()) {
				std::string object_filename;
				while (getline(file, object_filename)) { // Загружаем название файла обьекта
					if (object_filename.size() == 0) // Если размер строки 0
						continue;
					else if (object_filename[object_filename.find_first_not_of(" ")] == '#') // Или она начинается с символа для комментария
						continue; // То переходим к следующей строке

					object_filename = relative(object_filename).string(); // Приводм название файла к единому виду
					T* new_object = new T; // Создаем указатель на обьект

					if (!load_object_from_file(new_object, object_filename)) // Пробуем грузить из файла
						delete new_object; // Если не удалось, то удаляем и идем дальше
					else {
						resources.insert(pair<string, T*>(object_filename, new_object)); // Иначе закидываем в карту и тоже идем дальше
					}
				}
			}
			else {
				cout << "cannot load objects, file '" << filename << "' not found!" << endl;
			}
			file.close();
		}

		T* get_object(string filename) {
			filename = relative(filename).string(); // Приводм название файла к единому виду
			auto itrtr = resources.find(filename);

			if (itrtr == resources.end()) { // Если нужного обьекта нету, то попробуем его найти, но вообще лучше сразу сначала грузить
				T* new_object = new T; // Создаем указатель на обьект

				if (!load_object_from_file(new_object, filename)) { // Пробуем грузить из файла
					return nullptr;
				}
				else {
					resources.insert(pair<string, T*>(filename, new_object)); // Иначе закидываем в карту
					return new_object; // И возвращаем указатель на новый обьект
				}
			}
			else // Если обьект все же нашли, то возвращаем на него указатель
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


	// Класс для хранения анимаций в виде последовательности из пар <спрайт - время, которое будем его показывать>
	class Animation {
	private:
		vector<pair<Sprite, Time>> frames; // Хранит в себе спрайты и время от начала анимации до показа соответствующего спрайта

	public:
		Time full_animation_time = milliseconds(0);
		unsigned short number_of_frames = 0;

		float speed_koeff = 1.f; // Коэффициент скорости воспроизвдения анимаци

		void add_frame(Sprite _sprite, Time _time);

		Sprite* get_frame(Time _time);

		Time get_frame_time(unsigned short i);

		IntRect get_texture_rect(unsigned short i);

	};

	// Единый класс для спрайтов в игровом мире
	class Sprite_object {
	private:
		Sprite def_sprite;// Спрайт по умолчанию, когда не проигрывается никакая анимация и спрайт с картой высот
		Sprite* current_sprite;// Указатель на текущий кадр анимации (или кадр по умолчанию)

		double delta_layer, layer; // Смещение координаты для порядка отрисовки относительно нижней границы спрайта и сама координата отрисовки
		string texture_name, asset_name; // Необязательные параметры для хранения информации о родительской текстуре и ассете(для редактора)

		Clock animation_clock; // Таймер для отслеживания времени анимации

		bool visible; // Видимость спрайта
	public:
		vector<Animation> animations; // Вектор с анимациями обьекта
		Vector2d world_pos, world_size; // Позиция(левый верхний угол) и размер в игровом мире
		unsigned short id; // Тоже необязательная информация для хранения id спрайта
		Vector2u pixel_local_size; // Начальный размер спрайта в пикселях, без учета изменения масштаба
		short current_animation = -1; // Проигрываемая на данный момент анимация, -1 если спрайт по умолчанию
		bool active = true;

		// Обновление перменных, связанных с координатами и размерами спрайта, видимостью и слоем отрисовки
		bool var_update(Camera& cam);

		void animation_update();

		// Считывает спрайт из файла
		void load_from_file(ifstream& file);

		// Сохранение спрайта в файл
		void save_to_file(ofstream& file);

		// Вывести короткую информацию о спрайте в консоль
		void print_info();

		Sprite* get_sprite();

		double get_layer();

		void set_delta_layer(double _delta_layer);

		string get_texture_name();
	};

	// Класс для накладываемых эффектов
	class Effect {
	private:
		Shader* shader; // Указатель на шейдер данного эффекта

		vector<pair<string, float>> shader_attrs; // Атрибуты шейдера (могут быть только float)

		string shader_name;
		Clock clock;
	public:
		Sprite_object obj; // Обьект на основе которого будет создаваться эффект (например в нем может быть distortion texture)
		RenderTexture* effect_temp; // Указатель на промежуточную текстуру для рендера эффектов (она одна общая на все эффекты) НУЖНО УКАЗАТЬ ПРИ ЗАГРУЗКЕ
		Sprite s_effect_temp;

		void load_from_file(ifstream& file);

		// Создание спрайта эффекта на основе текстуры на которую эффект будет накладываться
		void draw(RenderTexture* texture, Camera cam);
	};

	// Класс для хранении информации об источниках света
	class Light_source {
	private:
		vector<pair<Vector3f, Time>> colors; // Зависимость цвета источника (и соответственно яркости) от времени
		bool smooth_brightness_change = false; // Плавное изменение яркости при анимации

		Time full_animation_time = milliseconds(0);
		Clock animation_clock;
	public:
		bool active = true; // Включен или выключен источник света вообще
		Vector2d world_pos; // Позиция источника света в игровом мире (середина)
		Vector2d world_size; // Размер области, которую охватывает свет
		bool visible = false; // Видимый ли свет от источника на экране
		Vector2f screen_pos, screen_size; // Оба этих значения нормаированы на размеры камеры для шейдера
		Vector2f direction; // Направление света(direction.x) и угол в котором светит источник(direction.y)
		Vector3f current_color;

		void var_update(Camera& cam);

		Vector3f animation_update();

		void load_from_file(ifstream& file);

		void print_info();
	};

	// Графический движок, обьединяющий в себе все остальные классы
	class Graphics_engine {
	private:
		const Color null_color = Color(0, 0, 0, 0);
		const int MAX_LIGHT_COUNT = 32;
		const int shader_update_freq = 20;
		Console* console = Console::get_instance();

		Clock clock; // Основные часы графического движка
		long frame_counter = 0; // Счетчик кадров

		// Очереди отрисовки для всех обьектов, эффектов
		deque<Sprite_object*> sprite_draw_order;
		deque<Effect*> effect_draw_order;
		deque<Sprite_object*> height_draw_order;

		Vector2u render_resolution, output_resolution; // Разрешение рендера и выходной картинки, уже отрисовывемой на экран

		Shader light_map_shader; // Шейдер для отрисовки карты осещения для источников света
		RenderTexture light_map; // Сама карта освещения

		Shader light_render_shader; // Шейдер для отрисовки теней по карте освещения
		Glsl::Vec4* light_positions; // Массивы с информацией об источниках света
		Glsl::Vec2* light_directions;
		Glsl::Vec3* light_colors;

		Shader combine_shader; // Шейдер для совмещения diffuse текстуры и отрисованным теням
		Shader posteffect_shader; // Шейдер с постэффектами
		Shader height_shader; // Шейдер для правильного наложения карт высот(НУЖНО ПРОВЕРИТЬ С ИСПОЛЬЗОАПНИЕМ TEXTURE_RECT В СПРАЙТАХ)

		Shader weather_shader; // Шейдер погоды
		Vector2f shader_wind_dist = Vector2f(0.f, 0.f); // Пройденный ветром путь, нужен для шейдера погоды
		Time wind_last_time;

		RenderTexture diffuse_result, height_result, weather_result, light_result, combined_result, posteffect_result; // Промежуточные текстуры при отрисовке
		Sprite output_sprite; // Выходной спрайт получаемый при работе "движка"

		RenderWindow* window;// Окно в которое будем отрисовывать итоговый спрайт

		void(*glTextureBarrier);

		void delete_sprite_object(vector<Sprite_object*>::iterator index);

		Clock fps_clock; // Часы для измерения производительности
		vector<float> frame_time; // Массив для времени кадра
		int frame_num_for_anylysis = 600; // Количество кадров для анализа фпс 
		float fps = 0.f, ten_percent_low = 0.f, one_percent_low = 0.f, sum_time = 0.f;
		Text fps_text;
	public:
		RenderTexture effect_temp; // Промежуточная текстура для эффектов
		Camera main_camera; // Основная камера, от лица который будет идти рендер

		// Погода и постэфеекты
		Vector3f ambient_light = Vector3f(0.7, 0.7, 0.8);

		Vector2f wind_speed = Vector2f(0.f, 0.f); // Скорость ветра
		float fog_def = 0.f; // Сила тумана по умолчанию
		float fog_cloud = 0.f; // Сила дополнительных облачков тумана
		float fog_height = 0.f; // Высота тумана
		float rain = 0.f; // Интенсивность дождя порядка ~[0.0, 1.5]

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

		vector<Sprite_object*> sprite_objects; // Обычные обьекты с diffuse текстурой
		vector<Sprite_object*> sprite_objects_for_deletion; // Список обьектов для удаления (они удаляются не сразу, а при отрисовке, чтобы не плодить лишних циклов)

		vector<Effect*> effects;// Список эффектов

		vector<Sprite_object*> height_objects;// Спрайты с картой высот вместо текстуры

		vector<Light_source*> lights;// Массив с источниками света

		// В конструкторе нужно указать окно, в которое рендерим итоговый результат и файл с настройками для движка
		Graphics_engine(RenderWindow& _window, ifstream& file);

		// Создаем очереди отрисовки
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