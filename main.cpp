#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <cstdint>
#include <vector>
#include <cmath>
#include <algorithm>
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

// Scientific color mapping function (like the reference)
sf::Color getScientificColor(float val, float minVal, float maxVal) {
    val = std::max(minVal, std::min(maxVal - 0.0001f, val));
    float d = maxVal - minVal;
    val = (d == 0.0f) ? 0.5f : (val - minVal) / d;
    
    float m = 0.25f;
    int num = static_cast<int>(val / m);
    float s = (val - num * m) / m;
    float r, g, b;
    
    switch (num) {
        case 0: r = 0.0f; g = s; b = 1.0f; break;      // Blue to Cyan
        case 1: r = 0.0f; g = 1.0f; b = 1.0f - s; break; // Cyan to Green
        case 2: r = s; g = 1.0f; b = 0.0f; break;      // Green to Yellow
        case 3: r = 1.0f; g = 1.0f - s; b = 0.0f; break; // Yellow to Red
    }
    
    return sf::Color(
        static_cast<uint8_t>(255 * r),
        static_cast<uint8_t>(255 * g),
        static_cast<uint8_t>(255 * b)
    );
}


int main() {
    // Create window
    int width = 100;  // Reduced from 800
    int height = 100; // Reduced from 600
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "Draggable Circle - SFML");
    window.setFramerateLimit(60);

    // Create fluid simulation
    float g = 9.81f;
    float density = 1.0f;
    float overrelax = 1.9f;
    Fluid *fluid_main = new Fluid(width, height, g, density, overrelax);

    // Set up boundary conditions - walls around the domain
    for(int i = 0; i < width + 2; i++) {
        for(int j = 0; j < height + 2; j++) {
            // Set all cells as fluid initially
            fluid_main->setFluid(i, j, 1);
            
            // Set boundary walls as solid
            if (i == 0 || i == width + 1 || j == 0 || j == height + 1) {
                fluid_main->setFluid(i, j, 0);  // 0 = solid
            }
        }
    }

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

        // Comment out line 144:
        // fluid_main->activateFluid();

        // Update fluid simulation frame by frame
        fluid_main->simulate(1.0f/60.0f, 20, g);
        
        // Get pressure field for visualization
        float* pressureField = fluid_main->getPressureField();
        
        
        // Clear window
        window.clear(sf::Color::White);

        // Scientific color mapping function (like the reference)
        auto getScientificColor = [](float val, float minVal, float maxVal) -> sf::Color {
            val = std::max(minVal, std::min(maxVal - 0.0001f, val));
            float d = maxVal - minVal;
            val = (d == 0.0f) ? 0.5f : (val - minVal) / d;
            
            float m = 0.25f;
            int num = static_cast<int>(val / m);
            float s = (val - num * m) / m;
            float r, g, b;
            
            switch (num) {
                case 0: r = 0.0f; g = s; b = 1.0f; break;      // Blue to Cyan
                case 1: r = 0.0f; g = 1.0f; b = 1.0f - s; break; // Cyan to Green
                case 2: r = s; g = 1.0f; b = 0.0f; break;      // Green to Yellow
            case 3: r = 1.0f; g = 1.0f - s; b = 0.0f; break; // Yellow to Red
            }
            
            return sf::Color(
                static_cast<uint8_t>(255 * r),
                static_cast<uint8_t>(255 * g),
                static_cast<uint8_t>(255 * b)
            );
        };

        // Draw pressure field visualization
        float cellWidth = 8.0f;  // Scale up for visualization (800/100 = 8)
        float cellHeight = 6.0f; // Scale up for visualization (600/100 = 6)
        
        // Find pressure range for proper normalization
        float minPressure = pressureField[0];
        float maxPressure = pressureField[0];
        for (int i = 0; i < (width + 2) * (height + 2); i++) {
            minPressure = std::min(minPressure, pressureField[i]);
            maxPressure = std::max(maxPressure, pressureField[i]);
        }
        
        for (int i = 1; i < width + 1; i++) {
            for (int j = 1; j < height + 1; j++) {
                int index = i * (height + 2) + j;  // Fix indexing to match Fluid class
                float pressure = pressureField[index];
                
                sf::RectangleShape cell;
                cell.setSize(sf::Vector2f(cellWidth, cellHeight));
                cell.setPosition(sf::Vector2f((i-1) * cellWidth, (j-1) * cellHeight));
                cell.setFillColor(getScientificColor(pressure, minPressure, maxPressure));
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