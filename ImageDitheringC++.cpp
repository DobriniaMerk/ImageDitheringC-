﻿#include "Utils.cpp"
#include <iostream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

int main()
{
    int colornum;
    std::string filename;

    sf::Image img;
    img.loadFromFile("img.png");

    sf::RenderWindow window(sf::VideoMode(img.getSize().x, img.getSize().y), "SFML works!");

    std::vector<sf::Color> colors = ImageDithering::Utils::Dither(img, 8);
    ImageDithering::Utils::SaveToFile(img, colors);
    img = ImageDithering::Utils::ReadFile();

    sf::Texture t;
    t.loadFromImage(img);
    sf::Sprite s;
    s.setTexture(t);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(s);
        window.display();
    }

    return 0;
}
