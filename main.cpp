#include <SFML/Graphics.hpp>
#include <iostream>

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
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "Draggable Circle - SFML");
    window.setFramerateLimit(60);

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

        // Clear window
        window.clear(sf::Color::White);

        // Draw circle
        circle.draw(window);

        // Display
        window.display();
    }

    return 0;
} 