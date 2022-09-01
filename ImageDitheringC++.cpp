#include "Utils.cpp"

int main()
{
    sf::Image img;
    img.loadFromFile("img2.png");

    sf::RenderWindow window(sf::VideoMode(img.getSize().x, img.getSize().y), "SFML works!");

    //std::vector<sf::Color> colors = ImageDithering::Utils::Dither(img, 8);
    std::vector<sf::Color> colors;
    colors.push_back(sf::Color(0, 0, 0));
    colors.push_back(sf::Color(255, 255, 255));
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