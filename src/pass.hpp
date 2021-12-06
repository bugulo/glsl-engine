#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <tuple>
#include <string>
#include <vector>

#include <GL/glew.h>

#include "engine.hpp"

class Engine;

class Pass
{
public:
    Pass(Engine *engine, int index);
    ~Pass();

    bool compile(std::string source);

    GLuint programId;

    std::vector<std::tuple<std::string, GLuint, GLenum>> textures;

    GLuint getProgramId();
    GLuint getFramebufferId();

    bool isCompute();
private:
    std::map<GLenum, GLuint> shaders;

    GLuint compile_shader(std::string shaderSource, GLenum type, std::string id);

    GLuint framebuffer = 0;

    int index;

    Engine *engine = nullptr;
};

#endif