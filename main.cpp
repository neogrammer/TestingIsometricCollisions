#include <SFML/Graphics.hpp>
#include <vector>
#include <array>
#include <memory>
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


struct Collider
{
    virtual ~Collider() {}

    virtual sf::Sprite& getSprite() = 0;
    virtual sf::Vector2f getPos() = 0;
    virtual sf::Vector2f getPoint(int idx_) = 0;
    virtual std::array<sf::Vector2f, 4> getPoints() = 0;
};

struct IsoTileCollider : public Collider
{
    sf::Sprite& spr;
    sf::Vector2f center{ 64.f, 32.f };
    std::vector<sf::Vector2f> vertex = {
        {64.f, 0.f},
        {0.f,-32.f},
        {-64.f,0.f}, 
        {0.f, 32.f}
    };	// "Model"

    IsoTileCollider() = delete;

    explicit IsoTileCollider(sf::Sprite& spr_)
        : spr{ spr_ } {
        
    }

    ~IsoTileCollider() {}

    IsoTileCollider(const IsoTileCollider&) = delete;
    IsoTileCollider& operator=(const IsoTileCollider&) = delete;
    IsoTileCollider(IsoTileCollider&&) = delete;
    IsoTileCollider& operator=(IsoTileCollider&&) = delete;

    sf::Sprite& getSprite() override
    {

        return spr;
    }

    sf::Vector2f getPos() override
    {
        return spr.getPosition() + center;
    }

    sf::Vector2f getPoint(int idx_) override
    {
        if (idx_ >= 4 || idx_ < 0)
            return this->getPos();
        else
            return this->getPos() + vertex[idx_];
    }

    std::array<sf::Vector2f, 4> getPoints() override
    {
        std::array<sf::Vector2f, 4> arr = { sf::Vector2f{0.f,0.f},  sf::Vector2f{0.f,0.f},  sf::Vector2f{0.f,0.f},  sf::Vector2f{0.f,0.f} };

        for (int i = 0; i < 4; i++)
        {
            arr[i] = getPoint(i);
        }
        return arr;
    }

};

struct BoxCollider : public Collider
{
    sf::Sprite& spr;
    sf::Vector2f center{ 30.f, 30.f };
    std::vector<sf::Vector2f> vertex = {
        { -30, -30 },
        { -30, +30 },
        { +30, +30 },
        { +30, -30 }
    };	// "Model"
    BoxCollider() = delete;
    explicit BoxCollider(sf::Sprite& spr_, float w_ = 60.f, float h_ = 60.f)
        : spr{ spr_ } {
        float hw = w_ / 2.f;
        float hh = h_ / 2.f;
        center.x = hw;
        center.y = hh;

        vertex[0].x = -hw;
        vertex[0].y = -hh;
        vertex[1].x = -hw;
        vertex[1].y = +hh;
        vertex[2].x = +hw;
        vertex[2].y = +hh;
        vertex[3].x = +hw;
        vertex[3].y = -hh;
    }

    ~BoxCollider() {}

   BoxCollider(const BoxCollider&) = delete;
   BoxCollider& operator=(const BoxCollider&) = delete;
   BoxCollider(BoxCollider&&) = delete;
   BoxCollider& operator=(BoxCollider&&) = delete;

   sf::Sprite& getSprite() override
   {

       return spr;
   }

   sf::Vector2f getPos() override
   {
       return spr.getPosition() + center;
   }

   sf::Vector2f getPoint(int idx_) override
   {
       if (idx_ >= 4 || idx_ < 0)
           return this->getPos();
       else
           return this->getPos() + vertex[idx_];
   }

   std::array<sf::Vector2f, 4> getPoints() override
   {
       std::array<sf::Vector2f, 4> arr = { sf::Vector2f{0.f,0.f},  sf::Vector2f{0.f,0.f},  sf::Vector2f{0.f,0.f},  sf::Vector2f{0.f,0.f} };

       for (int i = 0; i < 4; i++)
       {
           arr[i] = getPoint(i);
       }
       return arr;
   }

};

std::vector<std::unique_ptr<Collider>> collisionShapes;

// Use edge/diagonal intersections.
bool ShapeOverlap_DIAGS_STATIC(std::unique_ptr<Collider>& r1, std::unique_ptr<Collider>& r2)
{
    Collider* collider1 = r1.get();
    Collider* collider2 = r2.get();
    bool collided = false;
    for (int shape = 0; shape < 2; shape++)
    {
        if (shape == 1)
        {
            collider1 = r2.get();
            collider2 = r1.get();
        }

        // Check diagonals of this polygon...
        for (int p = 0; p < 4; p++)
        {
            sf::Vector2f line_r1s = collider1->getPos();

            auto points = collider1->getPoints();

            sf::Vector2f line_r1e = points[p];

            sf::Vector2f displacement = { 0,0 };

            // ...against edges of this polygon
            for (int q = 0; q < 4; q++)
            {
                auto points2 = collider2->getPoints();

                sf::Vector2f line_r2s = points2[q];
                sf::Vector2f line_r2e = points2[(q + 1) % points2.size()];

                // Standard "off the shelf" line segment intersection
                float h = (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r1e.y) - (line_r1s.x - line_r1e.x) * (line_r2e.y - line_r2s.y);
                float t1 = ((line_r2s.y - line_r2e.y) * (line_r1s.x - line_r2s.x) + (line_r2e.x - line_r2s.x) * (line_r1s.y - line_r2s.y)) / h;
                float t2 = ((line_r1s.y - line_r1e.y) * (line_r1s.x - line_r2s.x) + (line_r1e.x - line_r1s.x) * (line_r1s.y - line_r2s.y)) / h;

                if (t1 >= 0.0f && t1 < 1.0f && t2 >= 0.0f && t2 < 1.0f)
                {
                    collided = true;
                    displacement.x += (1.0f - t1) * (line_r1e.x - line_r1s.x);
                    displacement.y += (1.0f - t1) * (line_r1e.y - line_r1s.y);
                }
            }

            r1->getSprite().move({ displacement.x * (shape == 0 ? -1 : +1), displacement.y * (shape == 0 ? -1 : +1) });
        }
    }

    // Cant overlap if static collision is resolved
    return collided;
}

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
  
    collisionShapes.push_back(std::make_unique<BoxCollider>(playerShp, 50.f,30.f));

    std::vector<sf::Sprite> sprVec = {};

    int numTiles = 12 * 7;
    sprVec.reserve(numTiles);
    for (int i = 0; i < numTiles; i++)
    {
        int cx = i % 12;
        int cy = i / 12;

        float cposX = (float)cx * 128.f;
        float cposY = (float)cy * 64.f;

       
        sprVec.emplace_back(sf::Sprite{ tex });
        sprVec[i].setPosition(toIso({cposX,cposY}));
        collisionShapes.emplace_back(std::make_unique<IsoTileCollider>(sprVec[i]));
   
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
         
            // Check for overlap
       // Check for overlap
           // for (int m = 0; m < collisionShapes.size(); m++)
                for (int n = 1; n < collisionShapes.size(); n++)
                {
                     overlapHappened |= ShapeOverlap_DIAGS_STATIC(collisionShapes[0], collisionShapes[n]);
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