#include "Utils.cpp"

int main()
{
    sf::Image img;
    img.loadFromFile("img.png");
    sf::Color* colors = ImageDithering::Utils::Dither(img, 8);

    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");


    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.display();
    }

    return 0;
}