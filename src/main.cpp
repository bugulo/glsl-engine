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

    auto engine = Engine(512, 512);

    try {
        engine.init();
        // Upload shader to the engine
        engine.loadShader(std::string(argv[1]));
        // Update engine state while window opened
        while(!engine.shouldClose())
            engine.update();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    // Cleanup resources allocated by engine
    engine.destroy();
    return 0;
}