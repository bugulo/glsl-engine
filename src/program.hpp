#ifndef PROGRAM_H
#define PROGRAM_H

#include <map>
#include <tuple>
#include <string>
#include <vector>

#include <GL/glew.h>

#include "engine.hpp"

class Engine;
class Buffer;

class Program
{
public:
    /*!
     * @brief Program constructor
     * @param engine Engine instance
     * @param index Index of the Program
     */
    Program(Engine *engine, int index);

    //! Program destructor
    ~Program();

    //! Compile program with provided source
    void compile(std::string source);

    //! List of program's textures, it's type and location
    std::vector<std::tuple<GLuint, GLenum, GLuint>> textures;

    //! List of program's buffers and it's binding points
    std::vector<std::tuple<GLuint, int>> buffers;

    //! List of program's params
    std::map<std::string, std::string> params;

    //! Get OpenGL program ID
    GLuint getProgramId();

    //! Get OpenGL framebuffer ID
    GLuint getFramebufferId();

    //! Get OpenGL vertex array ID
    GLuint getVertexArrayId();
    

    //! Whether the program contains compute shader
    bool isCompute();

    //! Whether the program is ignored
    bool isIgnored = false;

    //! Whether the program should run only once
    bool isRanOnce = false;
private:
    //! Engine instance
    Engine *engine;
    
    //! Index of the program
    int index;

    //! Map of compiled shaders by it's type
    std::map<GLenum, GLuint> shaders;

    //! Parse program inputs and generate vertex array
    void parseProgramInputs();

    //! Parse program outputs and generate framebuffer
    void parseProgramOutputs();

    //! Parse program uniforms and generate textures
    void parseProgramUniforms();
    
    //! Parse program buffers and generate buffer objects
    void parseProgramBuffers();

    /*!
     * @brief Create and compile shader of the program
     * @param type Shader type
     * @param shaderSource Shader source
     * @param id Shader GLSL identificator
     * @return OpenGL shader ID
     */
    GLuint createShader(GLenum type, std::string shaderSource, std::string id);

    //! OpenGL program ID
    GLuint program = 0;

    //! OpenGL framebuffer ID
    GLuint framebuffer = 0;

    //! OpenGL vertex array ID
    GLuint varray = 0;
};

#endif