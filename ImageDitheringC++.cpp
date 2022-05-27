#include "Utils.cpp"

int main()
{
    sf::Image img;
    img.loadFromFile("img.png");
    sf::Color* colors = ImageDithering::Utils::Dither(img, 32);
    sf::Texture t;
    t.loadFromImage(img);
    sf::Sprite s;
    s.setTexture(t);

    sf::RenderWindow window(sf::VideoMode(img.getSize().x, img.getSize().y), "SFML works!");

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