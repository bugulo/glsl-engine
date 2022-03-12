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

class Pass
{
public:
    /*!
     * @brief Pass constructor
     * @param engine Engine instance
     * @param index Index of the pass
     */
    Pass(Engine *engine, int index);

    //! Pass destructor
    ~Pass();

    //! Compile pass with provided source
    void compile(std::string source);

    //! List of program's input textures, it's type and location
    std::vector<std::tuple<GLuint, GLenum, GLuint>> inputTextures;

    //! List of program's buffers and it's binding points
    std::vector<std::tuple<Buffer*, int>> buffers;

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

    //! Whether the pass is ignored
    bool isIgnored = false;

    //! Whether the pass should run only once
    bool isRanOnce = false;
private:
    //! Engine instance
    Engine *engine;
    
    //! Index of the pass
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
     * @brief Create and compile shader of the pass
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