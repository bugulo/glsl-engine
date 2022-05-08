#include "program.hpp"

#include <map>
#include <regex>
#include <string>
#include <sstream>
#include <stdexcept>

#include <GL/glew.h>

#include "engine.hpp"
#include "shaders.hpp"
#include "utils.hpp"

Program::Program(Engine *engine, int index)
{
    this->engine = engine;
    this->index = index;
}

Program::~Program()
{
    for(auto const& [type, id] : this->shaders)
        glDeleteShader(id);

    if(this->framebuffer != 0)
        glDeleteFramebuffers(1, &this->framebuffer);

    if(this->varray != 0)
        glDeleteVertexArrays(1, &this->varray);

    glDeleteProgram(this->program);
}

GLuint Program::createShader(GLenum type, std::string shaderSource, std::string id)
{
    std::stringstream buffer;
    buffer << engineShaderSource;
    buffer << mathShaderSource;
    buffer << "#define PROGRAM_" << std::to_string(this->index) << std::endl;
    buffer << "#define PROGRAM_" << std::to_string(this->index) << "_" << id << std::endl;
    buffer << shaderSource;

    auto string = buffer.str();
    auto source = string.c_str();

    this->engine->print("- Found %s shader, compiling...\n", id.c_str());

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check if shader was compiled successfully
    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE)
    {
        char error[512];
        glGetShaderInfoLog(shader, 512, NULL, error);
        throw std::runtime_error("Failed to compile shader, Reason: " + std::string(error));
    }

    return shader;
}

void Program::compile(std::string source)
{
    if(this->program != 0)
        throw std::runtime_error("Program was already compiled");

    if(this->params.contains("ONCE"))
        this->isRanOnce = true;
        
    // Compile shaders that are part of the program
    if(source.find("#ifdef PROGRAM_" + std::to_string(this->index) + "_COMPUTE_SHADER") != std::string::npos)
        this->shaders[GL_COMPUTE_SHADER] = this->createShader(GL_COMPUTE_SHADER, source, "COMPUTE_SHADER");

    if(source.find("#ifdef PROGRAM_" + std::to_string(this->index) + "_VERTEX_SHADER") != std::string::npos)
        this->shaders[GL_VERTEX_SHADER] = this->createShader(GL_VERTEX_SHADER, source, "VERTEX_SHADER");

    if(source.find("#ifdef PROGRAM_" + std::to_string(this->index) + "_FRAGMENT_SHADER") != std::string::npos)
        this->shaders[GL_FRAGMENT_SHADER] = this->createShader(GL_FRAGMENT_SHADER, source, "FRAGMENT_SHADER");

    if(source.find("#ifdef PROGRAM_" + std::to_string(this->index) + "_GEOMETRY_SHADER") != std::string::npos)
        this->shaders[GL_GEOMETRY_SHADER] = this->createShader(GL_GEOMETRY_SHADER, source, "GEOMETRY_SHADER");

    if(source.find("#ifdef PROGRAM_" + std::to_string(this->index) + "_TESS_CONTROL_SHADER") != std::string::npos)
        this->shaders[GL_TESS_CONTROL_SHADER] = this->createShader(GL_TESS_CONTROL_SHADER, source, "TESS_CONTROL_SHADER");

    if(source.find("#ifdef PROGRAM_" + std::to_string(this->index) + "_TESS_EVALUATION_SHADER") != std::string::npos)
        this->shaders[GL_TESS_EVALUATION_SHADER] = this->createShader(GL_TESS_EVALUATION_SHADER, source, "TESS_EVALUATION_SHADER");

    auto program = glCreateProgram();
    for(const auto &x : this->shaders)
        glAttachShader(program, x.second);
    glLinkProgram(program);
    
    // Check if program was linked successfully
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
    {
        char buffer[1024];
        glGetProgramInfoLog(program, 1024, 0, buffer);
        throw std::runtime_error("Failed to link program, Reason: " + std::string(buffer));
    }

    // Check if program was validated successfully
    GLint validateStatus;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
    if(validateStatus == GL_FALSE)
        throw std::runtime_error("Failed to validate program");

    this->program = program;

    // Parse programs for additional information
    this->parseProgramUniforms();
    this->parseProgramOutputs();
    this->parseProgramBuffers();
    this->parseProgramInputs();
}

void Program::parseProgramInputs()
{
    // Parse program inputs only if we work with vertex data
    if(this->isCompute())
        return;

    // User needs to specify vertex buffer so we can generate vertex array
    if(!this->params.contains("VBO"))
        throw std::runtime_error("VBO param is missing in PROGRAM_" + std::to_string(this->index));

    if(!this->engine->buffers.contains(this->params["VBO"]))
        throw std::runtime_error("Buffer referenced in VAO param does not exist");

    GLint inputCount; 
    glGetProgramInterfaceiv(program, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &inputCount);

    std::vector<std::tuple<GLint, GLint>> inputs;

    for(GLint i = 0; i < inputCount; i++)
    {
        GLsizei length;
        GLchar buffer[128] = {}; // TODO: we should fetch max length from gpu

        GLenum props[2] = {GL_TYPE, GL_LOCATION};
        GLint params[2] = {0};

        glGetProgramResourceName(program, GL_PROGRAM_INPUT, i, 128, &length, buffer);
        glGetProgramResourceiv(program, GL_PROGRAM_INPUT, i, 2, props, 2, nullptr, params);

        // Skip builtin inputs
        if(strcmp(buffer, "gl_VertexID") == 0 || strcmp(buffer, "gl_InstanceID") == 0 ||
           strcmp(buffer, "gl_DrawID") == 0|| strcmp(buffer, "gl_BaseVertex") == 0 ||
           strcmp(buffer, "gl_BaseInstance") == 0)
            continue;

        inputs.push_back(std::make_tuple(params[0], params[1]));
    }

    sort(inputs.begin(), inputs.end());

    glCreateVertexArrays(1, &this->varray);

    GLsizei currentStride = 0;
    for(auto const& [type, location] : inputs)
    {
        auto stride = Utils::getTypeSize(type);
        auto format = Utils::getTypeFormat(type);

        glEnableVertexArrayAttrib(this->varray, location);
        glVertexArrayAttribFormat(this->varray, location, std::get<0>(format), std::get<1>(format), GL_FALSE, currentStride);
        glVertexArrayAttribBinding(this->varray, location, 0);
        
        currentStride += stride;
    }

    glVertexArrayVertexBuffer(this->varray, 0, this->engine->buffers[this->params["VBO"]], 0, currentStride);

    // If element buffer was specified, bind it to vertex array
    if(this->params.contains("EBO"))
    {
        if(!this->engine->buffers.contains(this->params["EBO"]))
            throw std::runtime_error("Buffer referenced in EBO param does not exist");

        glVertexArrayElementBuffer(this->varray, this->engine->buffers[this->params["EBO"]]);
    }
}

void Program::parseProgramOutputs()
{
    // Parse program inputs only if we work with vertex data
    if(this->isCompute())
        return;

    // User needs to specify custom framebuffer creation
    if(!this->params.contains("CUSTOM_FRAMEBUFFER"))
        return;

    GLint outputCount; 
    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &outputCount);

    std::vector<std::tuple<GLuint, std::string>> outputs;
    for(GLint i = 0; i < outputCount; i++)
    {
        GLsizei length;
        GLchar buffer[128] = {}; // TODO: we should fetch max length from gpu

        glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, i, 128, &length, buffer);
        auto index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, buffer);
        outputs.push_back(std::make_tuple(index, std::string(buffer)));
    }

    glCreateFramebuffers(1, &this->framebuffer);

    for(auto const& [index, name] : outputs)
    {
        auto texture = this->engine->createTexture(name);
        glNamedFramebufferTexture(this->framebuffer, GL_COLOR_ATTACHMENT0 + index, texture, 0);
    }
}

void Program::parseProgramUniforms()
{
    GLint uniformCount;
    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);

    for(GLint i = 0; i < uniformCount; ++i) 
    {
        GLsizei length;
        GLint size;
        GLenum type;
        GLchar buffer[200] = {0}; // TODO: we should fetch max length from gpu

        glGetActiveUniform(program, i, 200, &length, &size, &type, buffer);

        if(type != GL_SAMPLER_2D && type != GL_IMAGE_2D)
            throw std::runtime_error("Unsupported uniform type");

        auto location = glGetUniformLocation(program, buffer);
        auto texture = this->engine->createTexture(std::string(buffer));
        this->textures.push_back(std::make_tuple(texture, type, location));
    }
}

void Program::parseProgramBuffers()
{
    GLint bufferCount;
    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &bufferCount);

    for(GLint i = 0; i < bufferCount; ++i) 
    {
        GLenum props[2] = {GL_BUFFER_BINDING, GL_BUFFER_DATA_SIZE};
        GLint params[2] = {0};

        GLchar buffer[201] = {}; // TODO: we should fetch max length from gpu
        glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, 201, nullptr, buffer);
        glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 2, props, 2, nullptr, params);

        // Skip builtin buffers
        if(strcmp(buffer, "EngineBuffer") == 0 || strcmp(buffer, "DrawCommandBuffer") == 0 ||
           strcmp(buffer, "WorkGroupBuffer") == 0)
            continue;

        this->buffers.push_back(std::make_tuple(engine->createBuffer(buffer, params[1]), params[0]));
    }
}

GLuint Program::getProgramId()
{
    return this->program;
}

GLuint Program::getFramebufferId()
{
    return this->framebuffer;
}

GLuint Program::getVertexArrayId()
{
    return this->varray;
}

bool Program::isCompute()
{
    return this->shaders.contains(GL_COMPUTE_SHADER);
}