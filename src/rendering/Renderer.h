#pragma once
#include <SFML/Graphics.hpp>
#include "ecs/World.h"

class Renderer {
public:
    void render(sf::RenderWindow& window, World& world, float alpha);
};
