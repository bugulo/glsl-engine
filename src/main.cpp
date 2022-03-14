#include <iostream>
#include <stdexcept>

#include "engine.hpp"

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        std::cout << "Invalid parameters" << std::endl;
        return 1;
    }

    auto engine = Engine();

    try {
        // Initialize engine with shader file
        engine.init(std::string(argv[1]));
        // Update engine state while window is open
        while(!engine.shouldClose())
            engine.update();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    // Cleanup resources allocated by the engine
    engine.destroy();
    return 0;
}