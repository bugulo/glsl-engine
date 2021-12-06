#include "pass.hpp"

#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

#include <stdio.h>

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
    printf("tessstt\n");
    for(auto const& [type, id] : this->shaders)
        glDeleteShader(id);
}

GLuint Pass::compile_shader(std::string shaderSource, GLenum type, std::string id)
{
    std::stringstream buffer;
    buffer << engineShaderSource;
    buffer << "#define PASS_" << std::to_string(this->index) << std::endl;
    buffer << "#define PASS_" << std::to_string(this->index) << "_" << id << std::endl << std::endl;
    buffer << shaderSource;
    auto string = buffer.str();
    auto source = string.c_str();

    printf("- Found %s shader, compiling...\n", id.c_str());

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        char error[512];
        glGetShaderInfoLog(shader, 512, NULL, error);
        printf("[!] Failed to compile shader %s\n", error);
    }

    return shader;
}

bool Pass::compile(std::string source)
{
    if(source.find("#ifdef PASS_" + std::to_string(this->index) + "_COMPUTE_SHADER") != std::string::npos)
        this->shaders[GL_COMPUTE_SHADER] = this->compile_shader(source, GL_COMPUTE_SHADER, "COMPUTE_SHADER");

    if(source.find("#ifdef PASS_" + std::to_string(this->index) + "_VERTEX_SHADER") != std::string::npos)
        this->shaders[GL_VERTEX_SHADER] = this->compile_shader(source, GL_VERTEX_SHADER, "VERTEX_SHADER");

    if(source.find("#ifdef PASS_" + std::to_string(this->index) + "_FRAGMENT_SHADER") != std::string::npos)
        this->shaders[GL_FRAGMENT_SHADER] = this->compile_shader(source, GL_FRAGMENT_SHADER, "FRAGMENT_SHADER");

    auto program = glCreateProgram();
    for(const auto &x : this->shaders)
        glAttachShader(program, x.second);
    glLinkProgram(program);
    
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[1024];
        glGetProgramInfoLog(program, 1024, 0, buffer);
        printf("%s\n", buffer);
        return false;
    }

    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status != GL_TRUE) {
        printf("%s\n", "Program failed to be validated");
        return false;
    }

    GLint uniforms;
    glGetProgramInterfaceiv(program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &uniforms);
    for(GLint i = 0; i < uniforms; ++i) 
    {
        GLint size;
        GLsizei length;
        GLenum type;
        GLchar buffer[201] = {0}; // we should fetch max length from gpu
        glGetActiveUniform(program, i, 200, &length, &size, &type, buffer);
        auto location = glGetUniformLocation(program, buffer);
        printf("%s, %d, %d, %d\n", buffer, location, type, GL_IMAGE_2D);
        
        auto name = std::string(buffer);
        auto id = this->engine->createTexture(name, 512, 512);
        this->textures.push_back(std::make_tuple(name, location, type));
    }

    GLint outputs; 
    glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &outputs);

    std::vector<std::tuple<std::string, GLuint>> textures;
    for(GLint i = 0; i < outputs; i++)
    {
        int identifier_length;
        char identifier[128];
        glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, i, 128, &identifier_length, identifier);
        GLuint output_index = glGetProgramResourceIndex(program, GL_PROGRAM_OUTPUT, identifier);

        printf("%s, %d\n", identifier, output_index);
        textures.push_back(std::make_tuple(std::string(identifier), output_index));
    }

    if(textures.size() == 1 && std::get<0>(textures[0]) == "defaultOutput")
        printf("default framebuffer\n");
    else if(textures.size() != 0)
    {
        printf("custom framebuffer\n");
        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        for(auto const& [name, location] : textures)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + location, GL_TEXTURE_2D, this->engine->createTexture(name, 500, 500), 0); 

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        this->framebuffer = fbo;
    }

    //for(const auto &x : this->shaders)
    //    glDeleteShader(x.second);

    programId = program;
    return true;
}

GLuint Pass::getProgramId()
{
    return this->programId;
}

GLuint Pass::getFramebufferId()
{
    return this->framebuffer;
}

bool Pass::isCompute()
{
    return this->shaders.contains(GL_COMPUTE_SHADER);
}