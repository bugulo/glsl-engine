#ifndef ENGINE_H
#define ENGINE_H

#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "pass.hpp"

#define MAX_PASS_COUNT 100

class Pass;
class Buffer;

struct InputBuffer
{
    //! Mouse X position
    int mouseX = 0;

    //! Mouse Y position
    int mouseY = 0;

    //! State of mouse buttons
    int btnState[GLFW_MOUSE_BUTTON_LAST] = {0};

    //! State of keyboard button
    int keyState[GLFW_KEY_LAST] = {0};
};

class Engine
{
public:
    /*!
     * @brief Engine constructor
     * @param width Window width
     * @param height Window height
     */
    Engine(int width, int height);

    //! Engine destructor
    ~Engine();

    /*!
     * @brief Upload shader into the engine and compile it 
     * @param filename Path to the shader file
     */
    void loadShader(std::string filename);

    //! Initialize engine
    void init();

    //! Update engine state (Run all passes one by one)
    void update();

    //! Free resources allocated by engine
    void destroy();

    //! Whether the window/context was closed
    bool shouldClose();

    //! Whether the engine standard output should be verbose
    bool verbose = false;

    //! Print message to standard output if verbose is turned on
    void print(const char *format, ...);

    /*!
     * @brief Create texture
     * @param name Texture name, format: name_$(sizeX)x$(sizeY) 
     * @return OpenGL texture ID
     */
    GLuint createTexture(std::string name);

    /*!
     * @brief Create buffer
     * @param name Buffer name
     * @param size Size of the buffer
     * @return Buffer instance
     */
    Buffer* createBuffer(std::string name, int size);

    //! Map of created textures and it's ids
    std::map<std::string, GLuint> textures;

    //! Map of created buffers and it's ids
    std::map<std::string, Buffer*> buffers;
private:
    //! Window width
    int width;

    //! Window height
    int height;

    //! OpenGL context
    GLFWwindow *context = nullptr;

    //! Input buffer instance
    InputBuffer inputBuffer;

    //! Key state change callback
    void key_callback(GLFWwindow *context, int key, int scancode, int action, int mods);

    //! Window size change callback
    void size_callback(GLFWwindow *context, int width, int height);

    //! Mouse position change callback
    void mouse_pos_callback(GLFWwindow *context, double xpos, double ypos);

    //! Mouse button state change callback
    void mouse_btn_callback(GLFWwindow *context, int button, int action, int mods);

    //! List of compiled pass instances
    std::vector<Pass*> passes;

    //! Input Buffer Object
    GLuint ibo;

    //! Work Group Buffer Object
    GLuint wgbo;

    //! Draw Command Buffer Object
    GLuint dcbo;
};

#endif