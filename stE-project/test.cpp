#pragma once
#include "Common.h"
#include "Console.h"
#include "Graphics.h"
#include "Weather.h"
#include "Car.h"
#include "LevelGenerator.h"

using namespace sf;
using namespace std;

int main()
{
    // Как работает тест: грузим пару спрайтов, машину и препятствие. Рендерим один кадр. Проверяем что в консоль не было напечатано ошибок и что проверка пересечения машины и препятствия работает
    RenderWindow window(VideoMode(VideoMode::getDesktopMode().size), "STE", Style::Fullscreen);
    Console* console = Console::get_instance();

    setlocale(LC_ALL, "");
    ifstream st("config/graphics.txt");
    gr::Graphics_engine eng(window, st);

    Vector2d def_size = Vector2d(9.0 * double(VideoMode::getDesktopMode().size.x) / double(VideoMode::getDesktopMode().size.y), 9.0);
    eng.main_camera.world_size = def_size;

    Game_time gtime(2022, 06, 18, 7);
    Weather_system wsys(&eng, &gtime);
    ifstream w("config/weather.txt");
    wsys.load_from_file(w);

    Car car(&eng);
    ifstream cr("config/car.txt");
    car.load_from_file(cr);

    Static_object obj("config/objects/campfire.txt", &eng);
    ifstream obst("config/objects/barrier.txt");
    Obstacle* barrier = new Barrier(obst, &eng);

    window.clear();
    eng.render(); // Тестовый рендер картинки
    window.display();

    bool restrict = barrier->check_restriction(car);

    window.close();
    cout << Console::get_instance()->any_errors || !restrict;
    return Console::get_instance()->any_errors || !restrict; // 0 - good, 1 - bad;
}
