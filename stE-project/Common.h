#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <map>
#include <deque>
#include <ctime>
#include <algorithm>
#include <vector>

using namespace sf;
using namespace std;

namespace sf {
	using Vector2d = Vector2<double>;
	using Vector3d = Vector3<double>;
	using DoubleRect = Rect<double>;
}

// Строка с текущей датой
string get_date_string();

// Строка с текущим временем
string get_time_string();

template<class T>
Vector2<T> normalized(Vector2<T> vec) {
	return vec / (sqrt(vec.x * vec.x + vec.y * vec.y));
}

template<class T>
ostream& operator<< (ostream& out, sf::Vector2<T> vec) {
	out << "(" << vec.x << ", " << vec.y << ")";
	return out;
}

template<class T>
ostream& operator<< (ostream& out, sf::Vector3<T> vec) {
	out << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return out;
}

template<class T>
ostream& operator<< (ostream& out, sf::Rect<T> rect) {
	out << "[" << rect.left << ", " << rect.top << ", " << rect.width << ", " << rect.height << "]";
	return out;
}

double try_stod(const string& str, double def_ans);

template<class T>
T smoothstep(T edge0, T edge1, T x)
{
	x = clamp((x - edge0) / (edge1 - edge0), T(0.0), T(1.0));
	return x * x * (3 - 2 * x);
}

class Game_time {
private:
	float time_ratio = 6.f; // Во сколько раз игровое время быстрее реального
	Clock clock; // Часы реального времени
	bool last_tick_running = true; // Шло ли время при предыдущем вызове update

	int months[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	string short_months[12] = { "Jan","Feb","Mar","Apr","May","Jun", "Jul","Aug","Sep","Oct","Nov","Dec" };

	void fix_time();

public:
	int year, month, day, hour, minute = 0, second = 0;

	int total_seconds = 0; // сколько секунд прошло с начал игры
	double total_hours = 0.0; // сколько часов прошло с начала игры 
	double day_hours = 0.0; // сколько часов прошло с начала суток

	bool running = true; // Идет ли время, или пауза

	Game_time(int start_year, int start_month, int start_day, int start_hour);

	void update();

	void skip_time(double hours);

	void save_to_file(ofstream& file);

	void load_from_file(ifstream& file);
};

