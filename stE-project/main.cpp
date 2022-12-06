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
    bool console_on = false;
    RenderWindow window(VideoMode(VideoMode::getDesktopMode().size), "STE", Style::Fullscreen);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
    //window.setFramerateLimit(60);
    Console* console = Console::get_instance();

    Resource_manager<Texture>* mng = Resource_manager<Texture>::get_instance();
    mng->preload("config/textures_preload.txt");

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

    LevelGenerator lvlgen("config/decor.txt", &eng, car);
    console->log("WASD - to move car, B - turn on/off engine, L - toggle lights, Arrow up/down - shift gears");

    Sprite car_hud(car.car_hud.getTexture());
    car_hud.setPosition(Vector2f(0.0, VideoMode::getDesktopMode().size.y - 200.0));

    bool game = true;
    Text text;
    Font font;
    font.loadFromFile("resources/console/font.ttf");
    text.setFont(font);

    while (window.isOpen()) {
        if (car.hp > 0.0 && !(car.fuel < 0.05 && car.speed.x < 0.05)) {
            game = true;
        }
        else {
            game = false;
            car.fuel = -1.0;
            car.acceleration.x = -3.0;
        }

        Event event;
        while (window.pollEvent(event)) {

            if (event.type == Event::Closed)
                window.close();

            else if (event.key.code == Keyboard::Escape) {
                window.close();
            }

            else if (event.type == event.KeyPressed && event.key.code == Keyboard::Tilde) {
                console_on = !console_on;
            }

            else if (game) {
                car.input(event);
            }
        }

        car.update();

        window.clear();
        eng.render();
        window.draw(car_hud);
        lvlgen.update();

        if (console_on) {
            window.draw(console->sprite);
        }
        if (!game) { // Проиграли, выводим счет
            text.setString("Game Over! Distance driven: " + to_string(int(car.distance_travel * 2.5)) + " meters");
            text.setPosition(Vector2f((VideoMode::getDesktopMode().size.x - text.getLocalBounds().width) * 0.5, (VideoMode::getDesktopMode().size.y - text.getLocalBounds().height) * 0.5));
            window.draw(text);
        }
        window.display();

        gtime.update();
        wsys.update();

        Mouse::setPosition(Vector2i(VideoMode::getDesktopMode().size.x / 2, VideoMode::getDesktopMode().size.y/2));
        eng.main_camera.world_pos = car.pos + Vector2d(car.speed.x / 5.0 + 3.0, 0.0);
        eng.main_camera.world_size = def_size * (clamp(car.speed.x / 100.0, 0.0, 0.15) + 1.0);
        //eng.main_camera.world_pos += Vector2d(0.003, 0);

    }
}