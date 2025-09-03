#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <memory>
#include "Colliders.h"
#include "util.h"
#include "Physics.h"
#include <iostream>
int main()
{

	sf::RenderWindow window{ sf::VideoMode({1600,900},32U), "Test Polygon Collide for Isometric maps" };
    sf::View vw = window.getDefaultView();
    vw.setCenter({ 400.f,200.f });
    window.setView(vw);

    sf::Texture tex("isoblock.png");
    sf::Texture ptex("pbox.png");

    sf::Sprite playerShp(ptex);
    playerShp.setPosition({ 600.f, -100.f });
    std::vector<sf::Sprite> sprVec = {};

    std::unique_ptr<Collider> playerCollider = std::make_unique<BoxCollider>(playerShp, 50.f, 30.f);
    std::vector<std::unique_ptr<Collider>> tiles;

    int numTiles = 12 * 7;
    sprVec.reserve(numTiles);
    tiles.reserve(numTiles);
    for (int i = 0; i < numTiles; i++)
    {
        int cx = i % 12;
        int cy = i / 12;
        float cposX = (float)cx * 128.f;
        float cposY = (float)cy * 64.f;   
        sprVec.emplace_back(sf::Sprite{ tex });
        sprVec[i].setPosition(util::toIso({cposX,cposY}));
        tiles.emplace_back(std::make_unique<IsoTileCollider>(sprVec[i]));
    }

    sf::Clock timer{};
    float elapsed{ 0.f };
    const float FPS60 = { 1.f / 60.f };
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();
            }
        }


        elapsed = timer.restart().asSeconds();

        bool overlapHappened = false;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            {
                playerShp.setPosition({ playerShp.getPosition().x,playerShp.getPosition().y - 300.f * elapsed});
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            {
                playerShp.setPosition({playerShp.getPosition().x - 300.f * elapsed , playerShp.getPosition().y });
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            {
                playerShp.setPosition({playerShp.getPosition().x, playerShp.getPosition().y + 300.f * elapsed });
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            {
                playerShp.setPosition({ playerShp.getPosition().x + 300.f * elapsed,playerShp.getPosition().y});
            }
   
            auto collidedIndices = Physics::HandleCollisions(*playerCollider, tiles);

            if (!collidedIndices.empty())
            {
                std::cout << "collision Occurred with " << collidedIndices.size() << " tiles" << std::endl;
            }

        // Remainder of main loop
        window.clear();
        for (int i = 0; i < sprVec.size(); i++)
        {
            window.draw(sprVec[i]);

        }
     
        window.draw(playerShp);
    
        window.display();
    }
	return 0;
}