#include "utils.hpp"

#include <stdexcept>

GLsizei Utils::getTypeSize(GLenum type)
{
    if(type == GL_FLOAT_VEC3)
        return 3 * sizeof(GL_FLOAT);
    else
        throw std::runtime_error("Unsupported GLSL type");
}

std::tuple<GLint, GLenum> Utils::getTypeFormat(GLenum type)
{
    if(type == GL_FLOAT_VEC3)
        return std::make_tuple(3, GL_FLOAT);
    else
        throw std::runtime_error("Unsupported GLSL type");
}