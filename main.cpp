#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <cstdint>
#include "Fluid.h"

class DraggableCircle {
private:
    sf::CircleShape shape;
    bool isDragging;
    sf::Vector2f dragOffset;

public:
    DraggableCircle(float radius, const sf::Vector2f& position, const sf::Color& color) 
        : isDragging(false) {
        shape.setRadius(radius);
        shape.setPosition(position);
        shape.setFillColor(color);
        shape.setOrigin(sf::Vector2f(radius, radius)); // Center the origin
    }

    void handleMouseDown(const sf::Vector2f& mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) {
            isDragging = true;
            dragOffset = mousePos - shape.getPosition();
        }
    }

    void handleMouseUp() {
        isDragging = false;
    }

    void handleMouseMove(const sf::Vector2f& mousePos) {
        if (isDragging) {
            shape.setPosition(mousePos - dragOffset);
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    bool isBeingDragged() const {
        return isDragging;
    }

    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
};

int main() {
    // Create window
    int width = 800;
    int height = 600;
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(width, height)), "Draggable Circle - SFML");
    window.setFramerateLimit(60);

    // Create fluid simulation
    float g = 9.81f;
    float density = 1.0f;
    float overrelax = 1.9f;
    Fluid *fluid_main = new Fluid(width, height, g, density, overrelax);

    // Create draggable circle
    DraggableCircle circle(30.0f, sf::Vector2f(400, 300), sf::Color::Red);

    // Main loop
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            // Window closed
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            // Mouse button pressed
            else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(
                        sf::Vector2i(mousePressed->position.x, mousePressed->position.y)
                    );
                    circle.handleMouseDown(mousePos);
                }
            }
            // Mouse button released
            else if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseReleased->button == sf::Mouse::Button::Left) {
                    circle.handleMouseUp();
                }
            }
            // Mouse moved
            else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if (circle.isBeingDragged()) {
                    sf::Vector2f mousePos = window.mapPixelToCoords(
                        sf::Vector2i(mouseMoved->position.x, mouseMoved->position.y)
                    );
                    circle.handleMouseMove(mousePos);
                }
            }
        }

        // Update fluid simulation frame by frame
        fluid_main->simulate(1.0f/60.0f, 20, g);
        
        // Get pressure field for visualization
        float* pressureField = fluid_main->getPressureField();

        // Clear window
        window.clear(sf::Color::White);

        // Draw pressure field visualization
        float cellWidth = 1.0f;
        float cellHeight = 1.0f;
        
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                int index = i * height + j;
                float pressure = pressureField[index];

                //std::cout << "Pressure: " << pressure << std::endl;
                
                // Normalize pressure to 0-1 range for visualization
                float normalizedPressure = std::max(0.0f, std::min(1.0f, pressure / 10.0f));
                
                sf::RectangleShape cell;
                cell.setSize(sf::Vector2f(cellWidth, cellHeight));
                cell.setPosition(sf::Vector2f(i * cellWidth, j * cellHeight));
                cell.setFillColor(sf::Color(
                    static_cast<uint8_t>(255 * (1.0f - normalizedPressure)),
                    static_cast<uint8_t>(255 * (1.0f - normalizedPressure)),
                    255
                ));
                window.draw(cell);
            }
        }

        // Draw circle
        circle.draw(window);

        // Display
        window.display();
    }

    return 0;
} 