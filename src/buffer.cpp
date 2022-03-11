#include "buffer.hpp"

#include <regex>

#include "engine.hpp"

Buffer::Buffer(Engine *engine, std::string name, int size)
{
    this->engine = engine;

    glCreateBuffers(1, &this->id);
    glNamedBufferData(this->id, size, NULL, GL_DYNAMIC_DRAW);
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &this->id);
}

void Buffer::bind(int point)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, point, this->id);
}

GLuint Buffer::getId()
{
    return this->id;
}