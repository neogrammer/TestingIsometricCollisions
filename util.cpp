#include "util.h"

sf::Vector2f util::toIso(sf::Vector2f cart)
{
    // cart is a position on the screen, lets strip it to cell space
    cart.x /= 128;
    cart.y /= 64;

    float xIso = (cart.x - cart.y) * (128.f / 2.f);
    float yIso = (cart.x + cart.y) * (64.f / 2.f);
    return { xIso,yIso };
}


sf::Vector2f util::toCart(sf::Vector2f iso)
{
    iso.x /= 128.f;
    iso.y /= 64.f;

    float xCart = (2 * iso.y + iso.x) * (64.f / 2.f);
    float yCart = (2 * iso.y - iso.x) * (64.f / 2.f);
    return { xCart,yCart };
}


