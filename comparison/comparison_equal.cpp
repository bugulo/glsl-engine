// Implementation of comparison.glsl without the engine

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <chrono>

const char *computeShaderSource = R"(
    #version 460

    layout(std430, binding = 0) buffer KeyBuffer {
        int pressedW;
        int pressedS;
    } keyBuffer;

    layout(std430, binding = 1) buffer ColorBuffer {
        int color;
    } colorBuffer;

    layout(local_size_x = 1) in;
    
    void main()
    {
        if(keyBuffer.pressedW == 1 && colorBuffer.color < 255)
            colorBuffer.color += 1;
        if(keyBuffer.pressedS == 1 && colorBuffer.color > 0)
            colorBuffer.color -= 1;
    }
)";

const char *vertexShaderSource = R"(
    #version 460

    layout (location = 0) in vec3 Position;

    void main()
    {
        gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);
    }
)";

const char *fragmentShaderSource = R"(
    #version 460

    layout(std430, binding = 1) buffer ColorBuffer {
        int color;
    } colorBuffer;

    out vec4 Output;

    void main() 
    {
        Output = vec4(colorBuffer.color / 255.0, colorBuffer.color / 255.0, colorBuffer.color / 255.0, 1.0);
    }
)";

int main()
{
    auto color = 100;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = glfwCreateWindow(512, 512, "Benchmark", NULL, NULL);

    if(window == NULL)
        throw std::runtime_error("Failed to initialize window");

    glfwMakeContextCurrent(window);

    if(glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize OpenGL");

    glViewport(0, 0, 512, 512);

    auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int compileStatus;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE)
        throw std::runtime_error("Failed to compile vertex shader");

    auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE)
        throw std::runtime_error("Failed to compile fragment shader");

    auto computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(computeShader);

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE)
        throw std::runtime_error("Failed to compile compute shader");

    auto program1 = glCreateProgram();
    glAttachShader(program1, computeShader);
    glLinkProgram(program1);

    int linkStatus;
    glGetProgramiv(program1, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
        throw std::runtime_error("Failed to link compute program");

    auto program2 = glCreateProgram();
    glAttachShader(program2, vertexShader);
    glAttachShader(program2, fragmentShader);
    glLinkProgram(program2);

    glGetProgramiv(program2, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
        throw std::runtime_error("Failed to link rendering program");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(computeShader);

    float vertices[] = {
        0.5f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };

    int keys[] = { 0, 0 };

    GLuint vertexBuffer, vertexArray, colorBuffer, keyBuffer;
    glCreateBuffers(1, &vertexBuffer);
    glCreateBuffers(1, &colorBuffer);
    glCreateBuffers(1, &keyBuffer);

    glNamedBufferData(vertexBuffer, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glNamedBufferData(colorBuffer, sizeof(GL_INT), &color, GL_DYNAMIC_DRAW);
    glNamedBufferData(keyBuffer, sizeof(keys), &keys, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, keyBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, colorBuffer);

    glCreateVertexArrays(1, &vertexArray);
    glEnableVertexArrayAttrib(vertexArray, 0);
    glVertexArrayAttribFormat(vertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vertexArray, 0, 0);
    glVertexArrayVertexBuffer(vertexArray, 0, vertexBuffer, 0, 3 * sizeof(GL_FLOAT));

    while(!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();

        keys[0] = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        keys[1] = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        glNamedBufferData(keyBuffer, sizeof(keys), &keys, GL_DYNAMIC_DRAW);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program1);
        glDispatchCompute(1, 1, 1);

        glUseProgram(program2);
        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Last frame execution time: " << duration.count() << " microseconds\n";
    }

    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &colorBuffer);
    glDeleteBuffers(1, &keyBuffer);
    glDeleteProgram(program1);
    glDeleteProgram(program2);

    glfwTerminate();
    return 0;
}