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
    
    float getRadius() const {
        return shape.getRadius();
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
    std::cout << "start fluid sim" << std::endl;
    
    // Create window
    int width = 100;  
    int height = 100;
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(800, 600)), "Draggable Circle - SFML");
    window.setFramerateLimit(30);
    
    std::cout << "Window created successfully" << std::endl;

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
    
    // Initialize smoke field
    for(int i = 0; i < width + 2; i++) {
        for(int j = 0; j < height + 2; j++) {
            fluid_main->setSmoke(i, j, 0.0f); // Initialize all cells with no smoke
        }
    }
    
    
    
    // Initialize smoke source and inflow velocity at left edge
    for(int j = 40; j < 60; j++) {
        fluid_main->setSmoke(1, j, 1.0f); // 100% density
        fluid_main->setU(1, j, 40.0f); 
    }

    // Create draggable circle
    DraggableCircle circle(30.0f, sf::Vector2f(400, 300), sf::Color::Red);

    // Set circle area as solid boundary in fluid simulation
    // Convert circle position from screen coordinates to grid coordinates
    float circleCenterX = circle.getPosition().x / 8.0f;  // cellWidth = 8.0f
    float circleCenterY = circle.getPosition().y / 6.0f;  // cellHeight = 6.0f
    float circleRadius = circle.getRadius() / 8.0f;       // Approximate radius in grid units
    
    // Set all cells within the circle as solid boundary
    for(int i = 1; i < width + 1; i++) {
        for(int j = 1; j < height + 1; j++) {
            // Calculate distance from cell center to circle center
            float dx = (i - 0.5f) - circleCenterX;
            float dy = (j - 0.5f) - circleCenterY;
            float distance = std::sqrt(dx*dx + dy*dy);
            
            // If cell is within circle radius, set as solid boundary
            if (distance <= circleRadius) {
                fluid_main->setFluid(i, j, 0);  // 0 = solid boundary
            }
        }
    }

    // Main loop
    int frame = 0;
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

        // Update circle boundary in fluid simulation when dragged
        // Convert circle position from screen coordinates to grid coordinates
        float circleCenterX = circle.getPosition().x / 8.0f;  // cellWidth = 8.0f
        float circleCenterY = circle.getPosition().y / 6.0f;  // cellHeight = 6.0f
        float circleRadius = circle.getRadius() / 8.0f;       // Approximate radius in grid units
        
        // Reset all interior cells to fluid first
        for(int i = 1; i < width + 1; i++) {
            for(int j = 1; j < height + 1; j++) {
                fluid_main->setFluid(i, j, 1);  // 1 = fluid
            }
        }
        
        // set all cells to s = 0 if contained within floor of radius circle
        for(int i = 1; i < width + 1; i++) {
            for(int j = 1; j < height + 1; j++) {
                // Calculate distance from cell center to circle center
                float dx = (i - 0.5f) - circleCenterX;
                float dy = (j - 0.5f) - circleCenterY;
                float distance = std::sqrt(dx*dx + dy*dy);
                
                // maybe add compensation factor to avoid boundary showing due to interpolation
                if (distance <= circleRadius) {
                    fluid_main->setFluid(i, j, 0);  // 0 = solid boundary
                }
            }
        }
        
        // Set inflow conditions
        for(int j = 45; j < 55; j++) {
            fluid_main->setSmoke(1, j, 1.0f); // Set smoke density to 1.0 at inflow
            fluid_main->setU(1, j, 200.0f); // Set inflow velocity to 20.0 at inflow
        }
        // Update fluid simulation frame by frame
        fluid_main->simulate(1.0f/60.0f, 20, g);
        frame++;
        
        // Get pressure field and smoke field for visualization
        float* pressureField = fluid_main->getPressureField();
        float* smokeField = fluid_main->getSmokeField();
        
        // Debug: Check smoke values at inflow
        if (frame == 0) {
            std::cout << "Initial smoke values at inflow:" << std::endl;
            for(int j = 40; j < 60; j++) {
                int index = 1 * (height + 2) + j;
                std::cout << "Smoke[" << 1 << "," << j << "] = " << smokeField[index] << std::endl;
            }
        }
        
        
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
        float cellWidth = 8.0f; 
        float cellHeight = 6.0f; 
        
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
                float smoke = smokeField[index];
                
                sf::RectangleShape cell;
                cell.setSize(sf::Vector2f(cellWidth, cellHeight));
                cell.setPosition(sf::Vector2f((i-1) * cellWidth, (j-1) * cellHeight));
                
                // Blend smoke (white) with pressure color (for efficiency; otherwise adnimation is very slow :(
                sf::Color pressureColor = getScientificColor(pressure, minPressure, maxPressure);
                uint8_t r = static_cast<uint8_t>(pressureColor.r * (smoke) + 255 * (1.0f - smoke));
                uint8_t g = static_cast<uint8_t>(pressureColor.g * (smoke) + 255 * (1.0f - smoke));
                uint8_t b = static_cast<uint8_t>(pressureColor.b * (smoke) + 255 * (1.0f - smoke));
                
                cell.setFillColor(sf::Color(r, g, b));
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