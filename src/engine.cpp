#include "engine.hpp"

#include <regex>
#include <cstdarg>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <GL/glew.h>

#include "pass.hpp"
#include "buffer.hpp"

Engine::Engine(int width, int height)
{
    this->width = width;
    this->height = height;

    this->verbose = true;
}

Engine::~Engine()
{
    this->destroy();
}

void Engine::init()
{
    if(this->context != nullptr)
        throw std::runtime_error("Context is already initialized");

    // Initialize GLFW
    if(glfwInit() == GLFW_FALSE)
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    this->context = glfwCreateWindow(this->width, this->height, "GPU Engine", NULL, NULL);

    if(this->context == NULL)
        throw std::runtime_error("Failed to initialize window");

    glfwSetWindowUserPointer(this->context, this);

    glfwSetFramebufferSizeCallback(context, [](GLFWwindow *context, int width, int height) {
        auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(context));
        engine->size_callback(context, width, height);
    });

    glfwSetKeyCallback(context, [](GLFWwindow *context, int key, int scancode, int action, int mods) {
        auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(context));
        engine->key_callback(context, key, scancode, action, mods);
    });

    glfwSetCursorPosCallback(context, [](GLFWwindow *context, double xpos, double ypos) {
        auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(context));
        engine->mouse_pos_callback(context, xpos, ypos);
    });

    glfwSetMouseButtonCallback(context, [](GLFWwindow *context, int button, int action, int mods) {
        auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(context));
        engine->mouse_btn_callback(context, button, action, mods);
    });

    glfwMakeContextCurrent(context);

    // Initialize OpenGL
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize OpenGL");

    glViewport(0, 0, this->width, this->height);
    
    // Generate built-in buffers

    glGenBuffers(1, &this->ibo); // Input Buffer Object
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->ibo);

    int workGroups[3] = {1, 1, 1};
    glGenBuffers(1, &this->wgbo); // Work Group Buffer Object
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, this->wgbo);
    glBufferData(GL_DISPATCH_INDIRECT_BUFFER, sizeof(workGroups), &workGroups, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->wgbo);

    glGenBuffers(1, &this->dcbo); // Draw Command Buffer Object
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, this->dcbo);
    glBufferStorage(GL_DRAW_INDIRECT_BUFFER, 20 * sizeof(unsigned int) * 5, nullptr, GL_MAP_WRITE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, this->dcbo);
}

void Engine::update()
{
    if(this->context == nullptr)
        throw std::runtime_error("Context is not initialized");

    glClear(GL_COLOR_BUFFER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ibo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(this->inputBuffer), &this->inputBuffer, GL_STATIC_READ);

    for(auto pass : this->passes)
    {
        if(pass->isIgnored)
            continue;

        glUseProgram(pass->getProgramId());

        for(auto const& [buffer, point] : pass->buffers)
            buffer->bind(point);

        if(pass->getFramebufferId() != 0)
            glBindFramebuffer(GL_FRAMEBUFFER, pass->getFramebufferId());

        int i = 0;
        for(auto const& [texture, type, location] : pass->inputTextures)
        {
            if(type == GL_IMAGE_2D)
            {
                glUniform1i(location, i);
                glBindImageTexture(i, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
            } 
            else if(type == GL_SAMPLER_2D)
            {
                glUniform1i(location, i);
                glBindTextureUnit(i, texture);
            }
                
            i++;
        }

        if(pass->isCompute() == true)
        {
            glDispatchComputeIndirect(0);
            glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        }
        else
        {
            if(pass->getVertexArrayId() != 0)
                glBindVertexArray(pass->getVertexArrayId());
            glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 1, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(0);

        if(pass->isRanOnce)
            pass->isIgnored = true;
    }
    
    glfwPollEvents();
    glfwSwapBuffers(this->context);
}

void Engine::destroy()
{
    if(this->context == nullptr)
        return;

    for(auto pass : this->passes)        
        delete pass;

    for(auto const& [name, buffer] : this->buffers)
        delete buffer;

    for(auto const& [name, id] : this->textures)
        glDeleteTextures(1, &id);

    glDeleteBuffers(1, &this->ibo);
    glDeleteBuffers(1, &this->wgbo);
    glDeleteBuffers(1, &this->dcbo);
    
    glfwDestroyWindow(this->context);
    glfwTerminate();

    this->context = nullptr;
}

bool Engine::shouldClose()
{
    if(this->context == nullptr)
        throw std::runtime_error("Context is not initialized");

    return glfwWindowShouldClose(this->context);
}

void Engine::key_callback(GLFWwindow *context, int key, int scancode, int action, int mods)
{
    if(action == 1 || action == 2)
        this->inputBuffer.keyState[key] = 1;
    else if(action == 0)
        this->inputBuffer.keyState[key] = 0;
}

void Engine::size_callback(GLFWwindow *context, int width, int height)
{
    this->width = width;
    this->height = height;
    glViewport(0, 0, width, height);
}

void Engine::mouse_pos_callback(GLFWwindow *context, double xpos, double ypos)
{
    this->inputBuffer.mouseX = xpos;
    this->inputBuffer.mouseY = ypos;
}

void Engine::mouse_btn_callback(GLFWwindow *context, int button, int action, int mods)
{
    this->inputBuffer.btnState[button] = action;
}

void Engine::loadShader(std::string filename)
{
    if(this->context == nullptr)
        throw std::runtime_error("Context is not initialized");

    std::ifstream stream(filename);

    if(stream.fail())
        throw std::invalid_argument("Could not open specified shader file");

    std::stringstream buffer;
    buffer << stream.rdbuf();

    for(size_t i = 0; i < MAX_PASS_COUNT; i++)
    {
        if(buffer.str().find("#ifdef PASS_" + std::to_string(i)) == std::string::npos)
            break;

        this->print("Compiling pass %zu\n", i);
        auto pass = new Pass(this, i);

        // Find all params of the pass
        std::smatch match;
        std::string haystack (buffer.str());
        while(std::regex_search(haystack, match, std::regex("#pragma PASS_" + std::to_string(i) + + "_PARAM (\\S+)(?:\\s(\\S+))?;")))
        {
            pass->params[match.str(1)] = match.str(2);
            haystack = match.suffix();
        }

        pass->compile(buffer.str());
        this->passes.push_back(pass);
    }
}

GLuint Engine::createTexture(std::string name)
{
    if(this->context == nullptr)
        throw std::runtime_error("Context is not initialized");

    if(this->textures.contains(name))
    {
        this->print("- Loaded texture %s with ID: %d\n", name.c_str(), this->textures[name]);
        return this->textures[name];
    }

    std::cmatch match;
    if(!std::regex_match(name.c_str(), match, std::regex("^\\S+_(\\d+)x(\\d+)$")))
        throw std::runtime_error("Failed to generate texture, Reason: Invalid texture name");
    
    this->print("- Creating texture: %s\n", name.c_str());

    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureStorage2D(texture, 1, GL_RGBA8, stoi(match[1]), stoi(match[2]));
    
    this->textures[name] = texture;
    this->print("- Loaded texture: %s with ID: %d\n", name.c_str(), texture);
    return texture;
}

Buffer* Engine::createBuffer(std::string name, int size)
{
    if(this->context == nullptr)
        throw std::runtime_error("Context is not initialized");
    
    if(this->buffers.contains(name))
    {
        this->print("- Loaded buffer %s with ID: %d\n", name.c_str(), this->buffers[name]->getId());
        return this->buffers[name];
    }

    std::cmatch match;
    if(!std::regex_match(name.c_str(), match, std::regex("^[a-zA-Z0-9]+(?:_(\\d+))?$")))
        throw std::runtime_error("Failed to generate buffer, Reason: Invalid buffer name");

    // If there is size specified in the buffer name, use that one instead of GLSL information
    size = match[1].matched ? stoi(match[1]) : size;

    this->print("- Creating buffer: %s (%db)\n", name.c_str(), size);

    auto buffer = new Buffer(this, name, size);
    this->buffers[name] = buffer;
    this->print("- Loaded buffer: %s with ID: %d\n", name.c_str(), buffer->getId());
    return buffer;
}

void Engine::print(const char *format, ...)
{
    if(!this->verbose)
        return;

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}