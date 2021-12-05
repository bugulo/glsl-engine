#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <string>
#include <vector>
#include <tuple>

#include <GL/glew.h>

class Program
{
public:
    Program();

    bool compile(std::string source, size_t pass);

    GLuint programId;

    bool isCompute;

    bool hasFramebuffer = false;
    GLuint framebufferId;

    std::vector<std::tuple<std::string, GLuint, GLenum>> myTextures;

    inline static std::map<std::string, GLuint> textureRegistry;
private:
    //bool validate_program(GLuint program);
    GLuint compile_shader(std::string shaderSource, size_t pass, GLenum type, std::string id);

    GLuint genTexture(std::string name);

    //std::vector<std::string> split(std::string string, std::string delimeter);
};

#endif