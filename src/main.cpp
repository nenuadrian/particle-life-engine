#include "application.h"

#include <iostream>
#include <stdexcept>

static constexpr int WIDTH = 1024;
static constexpr int HEIGHT = 1024;

int main() {
    Application app;

    try {
        app.init(WIDTH, HEIGHT, "Particle Life Engine");
        app.run();
        app.cleanup();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
