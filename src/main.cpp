#include <regex>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "program.hpp"

struct EngineBuffer 
{
    int workGroups[3] = {1, 1, 1};

    int keyState[GLFW_KEY_MENU];
};

EngineBuffer engine_buffer;

#define exit_with_error(code, string, ...) { fprintf(stderr, string, __VA_ARGS__); exit(code); }

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

    // COMPILE INPUT SHADER

    std::ifstream stream(computeShaderLocation);

    if(stream.fail())
        exit_with_error(1, "Failed to load specified shader file (%s)\n", computeShaderLocation);

    std::vector<Program> programs;

    std::stringstream shaderBuffer;
    shaderBuffer << stream.rdbuf();

    for(size_t i = 0; i < 500; i++)
    {
        //std::map<GLenum, GLuint> shaders;

        if(shaderBuffer.str().find("#ifdef PASS_" + std::to_string(i)) == std::string::npos)
            break;

        printf("Compiling pass %zu\n", i);
        auto program = Program();
        if(program.compile(shaderBuffer.str(), i))
            programs.push_back(program);
        else
            exit_with_error(1, "Failed to compile pass (%d)\n", i);
    }

    // GENERATE OUTPUT TEXTURE

    /*GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glClearTexImage(texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);*/

    // GENERATE BUFFERS

    GLuint ibo; // Input Buffer Object
    glGenBuffers(1, &ibo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ibo);

    GLuint wgbo; // Work Group Buffer Object
    glGenBuffers(1, &wgbo);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, wgbo);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(((EngineBuffer){0}).workGroups), &engine_buffer.workGroups, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, wgbo);

    GLuint dcbo; // Draw Command Buffer Object
    glGenBuffers(1, &dcbo);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, dcbo);
    glBufferStorage(GL_DRAW_INDIRECT_BUFFER, 20 * sizeof(unsigned int) * 5, nullptr, GL_MAP_WRITE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dcbo);

    GLuint vao;
    glGenVertexArrays(1, &vao); 
    glBindVertexArray(vao);

    GLuint vbo; // Vertex Buffer Object
    glGenBuffers(1, &vbo);  
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(unsigned int) * 100, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, vbo);

    GLuint ebo; // Element Buffer Object
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 100, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ebo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    /*unsigned int testt;
    glGenTextures(1, &testt);
    glBindTexture(GL_TEXTURE_2D, testt);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, testt, 0);*/

    while(!glfwWindowShouldClose(context))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ibo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(((EngineBuffer){0}).keyState), &engine_buffer.keyState, GL_STATIC_READ);

        /*auto program1 = programs[0];
        auto program2 = programs[1];
        auto program3 = programs[2];
        auto program4 = programs[3];

        auto texture1 = Program::textureRegistry["testTexture1"];
        auto texture2 = Program::textureRegistry["testTexture2"];
        auto texture3 = Program::textureRegistry["testTexture3"];

        glUseProgram(program1.programId);
        glDispatchComputeIndirect(0);
        glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glFinish();

        glUseProgram(program2.programId);
        glBindImageTexture(0, texture1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glUniform1i(0, 0);
        glBindImageTexture(1, texture2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glUniform1i(1, 1);
        glDispatchComputeIndirect(0);
        glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glFinish();

        glUseProgram(program3.programId);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glUniform1i(0, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glUniform1i(1, 1);
        glBindFramebuffer(GL_FRAMEBUFFER, program3.framebufferId);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glFinish();

        glUseProgram(program4.programId);
        glBindVertexArray(vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture3);
        glUniform1i(0, 0);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0);
        glFinish();*/

        for(auto const& program : programs)
        {
            glUseProgram(program.programId);

            if(program.hasFramebuffer)
                glBindFramebuffer(GL_FRAMEBUFFER, program.framebufferId);

            int i = 0;
            for(auto const& [name, location, type] : program.myTextures)
            {
                //printf("%s, %d\n", name.c_str(), Program::textureRegistry[name]);
                if(type == GL_IMAGE_2D)
                {
                    glUniform1i(location, i);
                    glBindImageTexture(i, Program::textureRegistry[name], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
                } 
                else if(type == GL_SAMPLER_2D)
                {
                    glUniform1i(location, i);
                    //glActiveTexture(GL_TEXTURE0 + i);
                    //glBindTexture(GL_TEXTURE_2D, Program::textureRegistry[name]);
                    glBindTextureUnit(i, Program::textureRegistry[name]);
                }
                    
                i++;
            }

            if(program.isCompute)
            {
                glDispatchComputeIndirect(0);
                glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
            else
            {
                glBindVertexArray(vao);
                glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0);
            }

            glBindFramebuffer(GL_FRAMEBUFFER , 0);
        }
        
        glfwPollEvents();
        glfwSwapBuffers(context);
    }

    for(auto const& program : programs)
    {
        glDeleteProgram(program.programId);
        if(program.hasFramebuffer)
            glDeleteFramebuffers(1, &program.framebufferId);
    }

    glDeleteVertexArrays(1, &vao);
    //glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &wgbo);
    glDeleteBuffers(1, &dcbo);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    
    glfwTerminate();
    return 0;
}