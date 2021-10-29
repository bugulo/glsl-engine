#include <regex>
#include <cstdio>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shaders.hpp"

struct EngineBuffer {
    int globalWorkGroups[3] = {1, 1, 1};

    int keyState[GLFW_KEY_MENU];
};

EngineBuffer engine_buffer;

#define exit_with_error(code, string, ...) { fprintf(stderr, string, __VA_ARGS__); exit(code); }

void validate_program(GLuint program) {
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

    std::stringstream buffer;
    buffer << stream.rdbuf();

    auto computeShaderString = buffer.str();
    computeShaderString = std::regex_replace(computeShaderString, std::regex("\\$\\{engineShader\\}"), engineShaderSource);
    auto computeShaderSource = computeShaderString.c_str();
    
    auto computeProgram = glCreateShaderProgramv(GL_COMPUTE_SHADER, 1, &computeShaderSource);
    validate_program(computeProgram);

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
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(((EngineBuffer){0}).globalWorkGroups), &engine_buffer.globalWorkGroups, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, wgbo);

    GLuint vao;
    glGenVertexArrays(1, &vao);

    while(!glfwWindowShouldClose(context))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // RUN COMPUTE SHADER
        glUseProgram(computeProgram);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ibo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(((EngineBuffer){0}).keyState), &engine_buffer.keyState, GL_STATIC_READ);

        glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glDispatchComputeIndirect(0);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
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
    glDeleteProgram(computeProgram);

    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &wgbo);
    
    glfwTerminate();
    return 0;
}