// Implementation of comparison.glsl without the engine using uniform variables instead of SSBOs

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <chrono>

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

    out vec4 Output;

    uniform int Color;

    void main() 
    {
        Output = vec4(Color / 255.0, Color / 255.0, Color / 255.0, 1.0);
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

    auto program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE)
        throw std::runtime_error("Failed to link program");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
        0.5f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };

    GLuint vertexBuffer, vertexArray;
    glCreateBuffers(1, &vertexBuffer);
    glNamedBufferData(vertexBuffer, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

    glCreateVertexArrays(1, &vertexArray);
    glEnableVertexArrayAttrib(vertexArray, 0);
    glVertexArrayAttribFormat(vertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vertexArray, 0, 0);
    glVertexArrayVertexBuffer(vertexArray, 0, vertexBuffer, 0, 3 * sizeof(GL_FLOAT));

    while(!glfwWindowShouldClose(window))
    {
        auto start = std::chrono::high_resolution_clock::now();

        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && color < 255)
            color += 1;
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && color > 0)
            color -= 1;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int location = glGetUniformLocation(program, "Color");

        glUseProgram(program);

        glUniform1i(location, color);
        glBindVertexArray(vertexArray);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();

        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Frame execution time: " << duration.count() << " microseconds\n";
    }

    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}