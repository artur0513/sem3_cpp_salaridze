#pragma once
#include "Common.h"


string get_date_string() {
	char buffer[80];
	time_t seconds = time(NULL);
	tm timeinfo;
	localtime_s(&timeinfo, &seconds);
	static const char* format = "%x";
	strftime(buffer, 80, format, &timeinfo);
	return string(buffer);
}

string get_time_string() {
	char buffer[80];
	time_t seconds = time(NULL);
	tm timeinfo;
	localtime_s(&timeinfo, &seconds);
	static const char* format = "%X";
	strftime(buffer, 80, format, &timeinfo);
	return string(buffer);
}

double try_stod(const string& str, double def_ans) {
	double d;
	try {
		d = stod(str);
	}
	catch (const std::invalid_argument&) {
		return def_ans;
	}
	catch (const std::out_of_range&) {
		return def_ans;
	}
	return d;
}

void Game_time::fix_time() {
	minute += second / 60;
	second %= 60;
	hour += minute / 60;
	minute %= 60;
	day += hour / 24;
	hour %= 24;

	if (day > months[month]) {
		day %= months[month];
		month++;
		if (month > 12) {
			month %= 12;
			year++;
		}
	}
}

Game_time::Game_time(int start_year, int start_month, int start_day, int start_hour) {
	year = start_year;
	month = start_month;
	day = start_day;
	hour = start_hour;
}

void Game_time::update() {
	if (!running) // Если часы остановлены то ничего не делаем
		return;

	if (running != last_tick_running) { // Есл часы не работали, а теперь их запустили то сбрасываем таймер (за время пока игровые часы не тикали там успевает накопится время)
		clock.restart();
		return;
	}

	if (clock.getElapsedTime().asSeconds() >= 1.f / time_ratio) {
		int seconds_passed = floor(clock.getElapsedTime().asSeconds() * time_ratio);
		second += seconds_passed;
		total_seconds += seconds_passed;
		total_hours = double(total_seconds) / 3600.0;
		day_hours = hour + double(minute * 60.0 + second) / 3600.0;
		clock.restart();
		fix_time();
	}
}

void Game_time::skip_time(double hours) {
	int delta_sec = int(hours * 3600.0);
	second += delta_sec;
	total_seconds += delta_sec;
	fix_time();
}

void Game_time::save_to_file(ofstream& file) {
	file << year << " " << month << " " << day << " " << hour << " " << second << " " << total_seconds << endl;
}

void Game_time::load_from_file(ifstream& file) {
	file >> year >> month >> day >> hour >> second >> total_seconds;
	total_hours = double(total_seconds) / 3600.0;
	day_hours = fmod(total_hours, 24.0);
}