#include "pass.hpp"

#include <map>
#include <regex>
#include <string>
#include <sstream>
#include <stdexcept>

#include <GL/glew.h>

#include "engine.hpp"
#include "shaders.hpp"

Pass::Pass(Engine *engine, int index)
{
    this->engine = engine;
    this->index = index;
}

Pass::~Pass()
{
    for(auto const& [type, id] : this->shaders)
        glDeleteShader(id);

    if(this->framebuffer != 0)
        glDeleteFramebuffers(1, &this->framebuffer);

    glDeleteProgram(this->program);
}

GLuint Pass::createShader(GLenum type, std::string shaderSource, std::string id)
{
    std::stringstream buffer;
    buffer << engineShaderSource;
    buffer << "#define PASS_" << std::to_string(this->index) << std::endl;
    buffer << "#define PASS_" << std::to_string(this->index) << "_" << id << std::endl;
    buffer << shaderSource;
    auto string = buffer.str();
    auto source = string.c_str();

    this->engine->print("- Found %s shader, compiling...\n", id.c_str());

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

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

void Pass::compile(std::string source)
{
    if(this->program != 0)
        throw std::runtime_error("Pass was already compiled");
        
    if(source.find("#ifdef PASS_" + std::to_string(this->index) + "_COMPUTE_SHADER") != std::string::npos)
        this->shaders[GL_COMPUTE_SHADER] = this->createShader(GL_COMPUTE_SHADER, source, "COMPUTE_SHADER");

    if(source.find("#ifdef PASS_" + std::to_string(this->index) + "_VERTEX_SHADER") != std::string::npos)
        this->shaders[GL_VERTEX_SHADER] = this->createShader(GL_VERTEX_SHADER, source, "VERTEX_SHADER");

    if(source.find("#ifdef PASS_" + std::to_string(this->index) + "_FRAGMENT_SHADER") != std::string::npos)
        this->shaders[GL_FRAGMENT_SHADER] = this->createShader(GL_FRAGMENT_SHADER, source, "FRAGMENT_SHADER");

    auto program = glCreateProgram();
    for(const auto &x : this->shaders)
        glAttachShader(program, x.second);
    glLinkProgram(program);
    
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
    {
        char buffer[1024];
        glGetProgramInfoLog(program, 1024, 0, buffer);
        throw std::runtime_error("Failed to link program, Reason: " + std::string(buffer));
    }

    GLint validateStatus;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
    if(validateStatus == GL_FALSE)
    {
        throw std::runtime_error("Failed to validate program");
    }

    GLint uniformCount;
    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniformCount);
    for(GLint i = 0; i < uniformCount; ++i) 
    {
        GLsizei length;
        GLint size;
        GLenum type;
        GLchar buffer[200] = {0}; // TODO: we should fetch max length from gpu

        glGetActiveUniform(program, i, 200, &length, &size, &type, buffer);
        auto location = glGetUniformLocation(program, buffer);
        
        auto name = std::string(buffer);

        auto texture = this->engine->createTexture(name);
        this->inputTextures.push_back(std::make_tuple(texture, type, location));
    }

    GLint outputCount; 
    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &outputCount);

    std::vector<std::tuple<GLuint, std::string>> outputs;
    for(GLint i = 0; i < outputCount; i++)
    {
        GLsizei length;
        GLchar buffer[128] = {};

        glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, i, 128, &length, buffer);
        GLuint location = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, buffer);
        outputs.push_back(std::make_tuple(location, std::string(buffer)));
    }

    if(outputs.size() != 0 && std::get<1>(outputs[0]) != "defaultOutput")
    {
        glGenFramebuffers(1, &this->framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, this->framebuffer);

        for(auto const& [location, name] : outputs)
        {
            auto texture = this->engine->createTexture(name);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + location, GL_TEXTURE_2D, texture, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    this->program = program;
}

GLuint Pass::getProgramId()
{
    return this->program;
}

GLuint Pass::getFramebufferId()
{
    return this->framebuffer;
}

bool Pass::isCompute()
{
    return this->shaders.contains(GL_COMPUTE_SHADER);
}