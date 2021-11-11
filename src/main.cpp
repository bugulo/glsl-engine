#include <regex>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shaders.hpp"

struct EngineBuffer 
{
    int workGroups[3] = {1, 1, 1};

    int keyState[GLFW_KEY_MENU];
};

enum ProgramType 
{
    DEFAULT = 0,
    COMPUTE = 1
};

EngineBuffer engine_buffer;

#define exit_with_error(code, string, ...) { fprintf(stderr, string, __VA_ARGS__); exit(code); }

void validate_program(GLuint program) 
{
    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[1024];
        glGetProgramInfoLog(program, 1024, 0, buffer);
        exit_with_error(1, "%s\n", buffer);
    }

    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status != GL_TRUE) {
        exit_with_error(1, "%s\n", "Program failed to be validated");
    }
}

GLuint compile_shader(std::string shaderSource, size_t pass, GLenum type, std::string id)
{
    std::stringstream buffer;
    buffer << engineShaderSource;
    buffer << "#define PASS_" << std::to_string(pass) << std::endl;
    buffer << "#define PASS_" << std::to_string(pass) << "_" << id << std::endl;
    buffer << shaderSource;
    auto source = buffer.str().c_str();

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
        exit_with_error(1, "[!] Failed to compile shader %s\n", error);
    }

    return shader;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *context, int key, int scancode, int action, int mods)
{
    if(action == 1 || action == 2)
        engine_buffer.keyState[key] = 1;
    else if(action == 0)
        engine_buffer.keyState[key] = 0;
}

int main(int argc, char **argv)
{
    if(argc != 2 )
        exit_with_error(1, "%s\n", "Invalid parameters");

    auto computeShaderLocation = argv[1];

    const int width = 512;
    const int height = 512;

    // INITIALIZE GLFW

    if(glfwInit() == GLFW_FALSE)
        exit_with_error(1, "%s\n", "Failed to initialize GLFW\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto context = glfwCreateWindow(width, height, "GPU Engine", NULL, NULL);

    if(context == NULL)
        exit_with_error(1, "%s\n", "Failed to create window\n");

    glfwSetKeyCallback(context, key_callback);
    glfwSetFramebufferSizeCallback(context, framebuffer_size_callback);

    glfwMakeContextCurrent(context);

    // INITIALIZE OPENGL

    if(glewInit() != GLEW_OK)
        exit_with_error(1, "%s\n", "Failed to initialize OpenGL\n");

    glViewport(0, 0, width, height);

    // INITIALIZE SHADERS & PROGRAMS

    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    auto textureProgram = glCreateProgram();
    glAttachShader(textureProgram, vertexShader);
    glAttachShader(textureProgram, fragmentShader);
    glLinkProgram(textureProgram);
    validate_program(textureProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::ifstream stream(computeShaderLocation);

    if(stream.fail())
        exit_with_error(1, "Failed to load specified shader file (%s)\n", computeShaderLocation);

    std::vector<GLuint> programs;

    std::stringstream shaderBuffer;
    shaderBuffer << stream.rdbuf();

    for(size_t i = 0; i < 500; i++)
    {
        std::map<GLenum, GLuint> shaders;

        if(shaderBuffer.str().find("#ifdef PASS_" + std::to_string(i)) == std::string::npos)
            break;
        
        printf("Compiling pass %zu\n", i);

        if(shaderBuffer.str().find("#ifdef PASS_" + std::to_string(i) + "_COMPUTE_SHADER") != std::string::npos)
            shaders[GL_COMPUTE_SHADER] = compile_shader(shaderBuffer.str(), i, GL_COMPUTE_SHADER, "COMPUTE_SHADER");

        if(shaderBuffer.str().find("#ifdef PASS_" + std::to_string(i) + "_VERTEX_SHADER") != std::string::npos)
            shaders[GL_COMPUTE_SHADER] = compile_shader(shaderBuffer.str(), i, GL_VERTEX_SHADER, "VERTEX_SHADER");

        if(shaderBuffer.str().find("#ifdef PASS_" + std::to_string(i) + "_FRAGMENT_SHADER") != std::string::npos)
            shaders[GL_COMPUTE_SHADER] = compile_shader(shaderBuffer.str(), i, GL_FRAGMENT_SHADER, "FRAGMENT_SHADER");

        auto program = glCreateProgram();
        for(const auto &x : shaders)
            glAttachShader(program, x.second);

        glLinkProgram(program);
        validate_program(program);
        programs.push_back(program);

        for(const auto &x : shaders)
            glDeleteShader(x.second);
    }

    // GENERATE OUTPUT TEXTURE

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glClearTexImage(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // GENERATE BUFFERS

    GLuint ibo; // Input Buffer Object
    glGenBuffers(1, &ibo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ibo);

    GLuint wgbo; // Work Group Buffer Object
    glGenBuffers(1, &wgbo);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, wgbo);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(((EngineBuffer){0}).workGroups), &engine_buffer.workGroups, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, wgbo);

    GLuint vao;
    glGenVertexArrays(1, &vao);

    while(!glfwWindowShouldClose(context))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ibo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(((EngineBuffer){0}).keyState), &engine_buffer.keyState, GL_STATIC_READ);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

        for(auto program : programs)
        {
            // RUN COMPUTE SHADER
            glUseProgram(program);

            glDispatchComputeIndirect(0);

            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        
        // DRAW COMPUTE SHADER OUTPUT
        glUseProgram(textureProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwPollEvents();   
        glfwSwapBuffers(context);
    }

    glDeleteProgram(textureProgram);

    for(auto program : programs)
        glDeleteProgram(program);

    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &wgbo);
    
    glfwTerminate();
    return 0;
}