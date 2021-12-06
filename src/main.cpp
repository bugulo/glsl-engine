#include <stdio.h>

#include "engine.hpp"

int main(int argc, char **argv)
{
    if(argc != 2 )
    {
        printf("Invalid parameters\n");
        return 1;
    }

    auto engine = Engine(512, 512);
    engine.init();
    engine.loadShader(std::string(argv[1]));
    while(!engine.shouldClose())
        engine.update();
    engine.destroy();
    return 0;
}