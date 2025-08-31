#include <SFML/Graphics.hpp>
#include <vector>

sf::Vector2f toIso(sf::Vector2f cart)
{
    // cart is a position on the screen, lets strip it to cell space
    cart.x /= 128;
    cart.y /= 64;

    float xIso = (cart.x - cart.y) * (128.f / 2.f);
    float yIso = (cart.x + cart.y) * (64.f / 2.f);
    return {xIso,yIso};
}


sf::Vector2f toCart(sf::Vector2f iso)
{
    iso.x /= 128.f;
    iso.y /= 64.f;

    float xCart = (2 * iso.y + iso.x) * (64.f / 2.f);
    float yCart = (2 * iso.y - iso.x) * (64.f / 2.f);
    return { xCart,yCart };
}


struct vec2d
{
    float x;
    float y;
};

struct polygon
{
    std::vector<vec2d> p;	// Transformed Points
    vec2d pos;				// Position of shape
    std::vector<vec2d> o;	// "Model" of shape							
    bool overlap = false;	// Flag to indicate if overlap has occurred
};

std::vector<polygon> vecShapes;

void CreateModel()
{
    // Create Diamond
    polygon s3;
    s3.pos = { 0, 0 };
    s3.o.push_back({ 64, 0 });
    s3.o.push_back({ 0, -32 });
    s3.o.push_back({ -64, 0 });
    s3.o.push_back({ 0, 32 });
    s3.p.resize(4);

    vecShapes.push_back(s3);
}


// Use edge/diagonal intersections.
bool ShapeOverlap_DIAGS_STATIC(polygon& r1, polygon& r2)
{
    polygon* poly1 = &r1;
    polygon* poly2 = &r2;

    for (int shape = 0; shape < 2; shape++)
    {
        if (shape == 1)
        {
            poly1 = &r2;
            poly2 = &r1;
        }

        // Check diagonals of this polygon...
        for (int p = 0; p < poly1->p.size(); p++)
        {
            vec2d line_r1s = poly1->pos;
            vec2d line_r1e = poly1->p[p];

            vec2d displacement = { 0,0 };

            // ...against edges of this polygon
            for (int q = 0; q < poly2->p.size(); q++)
            {
                vec2d line_r2s = poly2->p[q];
                vec2d line_r2e = poly2->p[(q + 1) % poly2->p.size()];

                // Standard "off the shelf" line segment intersection
                float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) - (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
                float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) + (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
                float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) + (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

                if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
                {
                    displacement.x += (1.0f - t1) * (line_r1e.x - line_r1s.x);
                    displacement.y += (1.0f - t1) * (line_r1e.y - line_r1s.y);
                }
            }

            r1.pos.x += displacement.x * (shape == 0 ? -1 : +1);
            r1.pos.y += displacement.y * (shape == 0 ? -1 : +1);
        }
    }

    // Cant overlap if static collision is resolved
    return false;
}



int main()
{

	sf::RenderWindow window{ sf::VideoMode({1600,900},32U), "Test Polygon Collide for Isometric maps" };
    sf::View vw = window.getDefaultView();
    vw.setCenter({ 400.f,200.f });
    window.setView(vw);
    
    sf::RectangleShape playerShp{ { 299.f, 240.f } };
    playerShp.setPosition({ 600.f, -100.f });
    sf::Vector2f colBoxOffset{ 109.f,200.f };
    playerShp.setOutlineColor(sf::Color::White);
    playerShp.setOutlineThickness(2);
    playerShp.setFillColor(sf::Color::Transparent);


   
    polygon r1;

    r1.pos.x = 400.f;
    r1.pos.y = 0.f;
    r1.o.push_back({ -30, -30 });
    r1.o.push_back({ -30, +30 });
    r1.o.push_back({ +30, +30 });
    r1.o.push_back({ +30, -30 });
    r1.p.resize(4);

    vecShapes.push_back(r1);
   

    

    sf::RectangleShape collideShp({ 60.f, 60.f });
    collideShp.setFillColor(sf::Color::Yellow);
    collideShp.setOrigin({ 30.f,30.f });

   
    
    
    sf::Texture tex("isoblock.png");
    
    


    std::vector<sf::Sprite> sprVec = {};

    int numTiles = 12 * 7;
    for (int i = 0; i < numTiles; i++)
    {
        int cx = i % 12;
        int cy = i / 12;

        float cposX = (float)cx * 128.f;
        float cposY = (float)cy * 64.f;

        sprVec.push_back(sf::Sprite(tex));
        sprVec[i].setPosition(toIso({cposX,cposY}));
        CreateModel();
        vecShapes[i+1].pos = { sprVec[i].getPosition().x + 64.f, sprVec[i].getPosition().y + 32.f };
    }
   

    //spr.setPosition({ 0.f, 0.f });
    


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
               vecShapes[0].pos = {vecShapes[0].pos.x,vecShapes[0].pos.y -300.f * elapsed};
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            {
                vecShapes[0].pos = { vecShapes[0].pos.x - 300.f * elapsed ,vecShapes[0].pos.y};
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            {
                vecShapes[0].pos = { vecShapes[0].pos.x,vecShapes[0].pos.y + 300.f * elapsed };
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            {
                vecShapes[0].pos = { vecShapes[0].pos.x + 300.f * elapsed,vecShapes[0].pos.y };
            }
           // playerShp.setPosition(collideShp.getPosition() - colBoxOffset);


            // Update Shapes and reset flags
            for (auto& r : vecShapes)
            {
                for (int i = 0; i < r.o.size(); i++)
                    r.p[i] =
                {	// 2D Rotation Transform + 2D Translation
                    r.o[i].x + r.pos.x,
                    r.o[i].y + r.pos.y,
                };

                r.overlap = false;
            }



           
            // Check for overlap
       // Check for overlap
            for (int m = 0; m < vecShapes.size(); m++)
                for (int n = m + 1; n < vecShapes.size(); n++)
                {
                     overlapHappened |= ShapeOverlap_DIAGS_STATIC(vecShapes[m], vecShapes[n]); break;
                }




            
        


        // Remainder of main loop
        window.clear();
        for (int i = 0; i < sprVec.size(); i++)
        {
            window.draw(sprVec[i]);

        }
       /* for (int i = 0; i < sprVec.size(); i++)
        {
            sf::RectangleShape s{ {128.f,64.f} };
            s.setOutlineColor(sf::Color::Red);
            s.setOutlineThickness(1);
            s.setFillColor(sf::Color::Transparent);
            s.setPosition({ vecShapes[i].pos.x - 64.f,vecShapes[i].pos.y - 32.f });
            window.draw(s);

        }*/
        collideShp.setPosition({ vecShapes[0].pos.x, vecShapes[0].pos.y });
        playerShp.setPosition(collideShp.getPosition() - colBoxOffset);

        window.draw(playerShp);
        window.draw(collideShp);

        window.display();
    }
	return 0;
}