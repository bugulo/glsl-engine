#include "program.hpp"

#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

#include <stdio.h>

#include "shaders.hpp"

Program::Program()
{
}

GLuint Program::compile_shader(std::string shaderSource, size_t pass, GLenum type, std::string id)
{
    std::stringstream buffer;
    buffer << engineShaderSource;
    buffer << "#define PASS_" << std::to_string(pass) << std::endl;
    buffer << "#define PASS_" << std::to_string(pass) << "_" << id << std::endl << std::endl;
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

bool Program::compile(std::string source, size_t pass)
{
    std::map<GLenum, GLuint> shaders;

    if(source.find("#ifdef PASS_" + std::to_string(pass) + "_COMPUTE_SHADER") != std::string::npos)
        shaders[GL_COMPUTE_SHADER] = this->compile_shader(source, pass, GL_COMPUTE_SHADER, "COMPUTE_SHADER");

    if(source.find("#ifdef PASS_" + std::to_string(pass) + "_VERTEX_SHADER") != std::string::npos)
        shaders[GL_VERTEX_SHADER] = this->compile_shader(source, pass, GL_VERTEX_SHADER, "VERTEX_SHADER");

    if(source.find("#ifdef PASS_" + std::to_string(pass) + "_FRAGMENT_SHADER") != std::string::npos)
        shaders[GL_FRAGMENT_SHADER] = compile_shader(source, pass, GL_FRAGMENT_SHADER, "FRAGMENT_SHADER");

    auto program = glCreateProgram();
    for(const auto &x : shaders)
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
        auto id = genTexture(name);
        myTextures.push_back(std::make_tuple(name, location, type));
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
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + location, GL_TEXTURE_2D, genTexture(name), 0); 

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        framebufferId = fbo;
        hasFramebuffer = true;
    }

    /*GLint buffers;
    glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &buffers);
    for(GLint i = 0; i < buffers; ++i) 
    {
        GLchar buffer[201] = {}; // we should fetch max length from gpu
        glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, 201, nullptr, buffer);
        GLint binding;
        GLenum bindingE = GL_BUFFER_BINDING;
        glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, &bindingE, 1, nullptr, &binding);

        GLint size;
        GLenum sizeE = GL_BUFFER_DATA_SIZE;
        glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, &sizeE, 1, nullptr, &size);

        GLint active;
        GLenum activeE = GL_NUM_ACTIVE_VARIABLES;
        glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, 1, &activeE, 1, nullptr, &active);
        printf("%s %d %d %d\n", buffer, binding, size, active);

        std::cmatch match;
        if(std::regex_match(buffer, match, std::regex("^vao(\\d+)_vertex((?:_(?:\\d+f))+)$")))
        {
            auto vao_id = stoi(match[1].str());

            GLuint vao;
            glGenVertexArrays(1, &vao); 
            glBindVertexArray(vao);

            auto params = this->split(match[2].str(), "_");
            for(const auto &param : params)
            {
                std::cmatch match2;
                if(!std::regex_match(param.c_str(), match2, std::regex("^(\\d+)(f|u)$")))
                    continue;

                auto number = stoi(match2[1].str());
                auto type = match2[2].str();

                printf("%d, %s\n", number, type.c_str());
            }
        }
    }*/

    isCompute = shaders.contains(GL_COMPUTE_SHADER);

    for(const auto &x : shaders)
        glDeleteShader(x.second);

    programId = program;
    return true;
}

GLuint Program::genTexture(std::string name)
{
    if(textureRegistry.contains(name))
    {
        printf("Loaded texture %s with ID: %d\n", name.c_str(), textureRegistry[name]);
        return textureRegistry[name];
    }

    printf("Generating texture %s\n", name.c_str());

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, GL_RGBA8, 800, 600);
    
    textureRegistry[name] = texture;
    printf("Loaded texture %s with ID: %d\n", name.c_str(), texture);
    return texture;
}

/*std::vector<std::string> Program::split(std::string string, std::string delimeter)
{
    std::vector<std::string> strings;

    char *str = (char *) string.c_str();
    char *pch = strtok(str, delimeter.c_str());
    while (pch != NULL)
    {
        strings.push_back(std::string(pch));
        pch = strtok(NULL, delimeter.c_str());
    }

    return strings;
}*/