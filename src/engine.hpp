#ifndef ENGINE_H
#define ENGINE_H

#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "pass.hpp"

#define MAX_PASS_COUNT 100

struct EngineBuffer 
{
    int workGroups[3] = {1, 1, 1};

    int keyState[GLFW_KEY_MENU];
};

class Pass;

class Engine
{
public:
    Engine(int width, int height);
    ~Engine();

    void loadShader(std::string filename);

    void init();
    void update();
    void destroy();

    bool shouldClose();

    bool verbose = false;

    void print(const char *format, ...);

    GLuint createTexture(std::string name, int width, int height);
private:
    int width;
    int height;

    GLFWwindow *context = nullptr;

    EngineBuffer buffer;

    void key_callback(GLFWwindow *context, int key, int scancode, int action, int mods);

    std::vector<Pass*> passes;

    GLuint ibo;
    GLuint wgbo;
    GLuint dcbo;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    std::map<std::string, GLuint> textures;
};

#endif