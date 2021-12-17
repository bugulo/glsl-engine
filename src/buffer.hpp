#ifndef BUFFER_H
#define BUFFER_H

#include <string>

#include <GL/glew.h>

class Engine;

class Buffer
{
public:
    /*!
     * @brief Buffer constructor
     * @param engine Engine instance
     * @param name Buffer name
     * @param size Size of the buffer
     */
    Buffer(Engine *engine, std::string name, int size);

    //! Buffer destructor
    ~Buffer();

    /*!
     * @brief Bind the buffer on specific binding point
     * @param point Binding point
     */
    void bind(int point);

    //! Get OpenGL buffer ID
    GLuint getId();
private:
    //! Engine instance
    Engine *engine;

    //! OpenGL buffer ID
    GLuint id = 0;
};

#endif