#include "utils.hpp"

#include <stdexcept>

GLsizei Utils::getTypeSize(GLenum type)
{
    // Scalar types
    if(type == GL_FLOAT)
        return sizeof(GL_FLOAT);
    else if(type == GL_DOUBLE)
        return sizeof(GL_DOUBLE);
    else if(type == GL_INT)
        return sizeof(GL_INT);
    else if(type == GL_UNSIGNED_INT)
        return sizeof(GL_UNSIGNED_INT);
    else if(type == GL_BOOL)
        return sizeof(GL_BOOL);

    // Float vectors
    else if(type == GL_FLOAT_VEC4)
        return 4 * sizeof(GL_FLOAT);
    else if(type == GL_FLOAT_VEC3)
        return 3 * sizeof(GL_FLOAT);
    else if(type == GL_FLOAT_VEC2)
        return 2 * sizeof(GL_FLOAT);

    // Double vectors
    else if(type == GL_DOUBLE_VEC4)
        return 4 * sizeof(GL_DOUBLE);
    else if(type == GL_DOUBLE_VEC3)
        return 3 * sizeof(GL_DOUBLE);
    else if(type == GL_DOUBLE_VEC2)
        return 2 * sizeof(GL_DOUBLE);

    // Integer vectors
    else if(type == GL_INT_VEC4)
        return 4 * sizeof(GL_INT);
    else if(type == GL_INT_VEC3)
        return 3 * sizeof(GL_INT);
    else if(type == GL_INT_VEC2)
        return 2 * sizeof(GL_INT);

    // Unsigned integer vectors
    else if(type == GL_UNSIGNED_INT_VEC4)
        return 4 * sizeof(GL_UNSIGNED_INT);
    else if(type == GL_UNSIGNED_INT_VEC3)
        return 3 * sizeof(GL_UNSIGNED_INT);
    else if(type == GL_UNSIGNED_INT_VEC2)
        return 2 * sizeof(GL_UNSIGNED_INT);

    // Bool vectors
    else if(type == GL_BOOL_VEC4)
        return 4 * sizeof(GL_BOOL);
    else if(type == GL_BOOL_VEC3)
        return 3 * sizeof(GL_BOOL);
    else if(type == GL_BOOL_VEC2)
        return 2 * sizeof(GL_BOOL);
    else
        throw std::runtime_error("Unsupported GLSL type");
}

std::tuple<GLint, GLenum> Utils::getTypeFormat(GLenum type)
{
    // Scalar types
    if(type == GL_FLOAT)
        return std::make_tuple(1, GL_FLOAT);
    else if(type == GL_DOUBLE)
        return std::make_tuple(1, GL_DOUBLE);
    else if(type == GL_INT)
        return std::make_tuple(1, GL_INT);
    else if(type == GL_UNSIGNED_INT)
        return std::make_tuple(1, GL_UNSIGNED_INT);
    else if(type == GL_BOOL)
        return std::make_tuple(1, GL_BOOL);

    // Float vectors
    else if(type == GL_FLOAT_VEC4)
        return std::make_tuple(4, GL_FLOAT);
    else if(type == GL_FLOAT_VEC3)
        return std::make_tuple(3, GL_FLOAT);
    else if(type == GL_FLOAT_VEC2)
        return std::make_tuple(2, GL_FLOAT);

    // Double vectors
    else if(type == GL_DOUBLE_VEC4)
        return std::make_tuple(4, GL_DOUBLE);
    else if(type == GL_DOUBLE_VEC3)
        return std::make_tuple(3, GL_DOUBLE);
    else if(type == GL_DOUBLE_VEC2)
        return std::make_tuple(2, GL_DOUBLE);

    // Integer vectors
    else if(type == GL_INT_VEC4)
        return std::make_tuple(4, GL_INT);
    else if(type == GL_INT_VEC3)
        return std::make_tuple(3, GL_INT);
    else if(type == GL_INT_VEC2)
        return std::make_tuple(2, GL_INT);

    // Unsigned integer vectors
    else if(type == GL_UNSIGNED_INT_VEC4)
        return std::make_tuple(4, GL_UNSIGNED_INT);
    else if(type == GL_UNSIGNED_INT_VEC3)
        return std::make_tuple(3, GL_UNSIGNED_INT);
    else if(type == GL_UNSIGNED_INT_VEC2)
        return std::make_tuple(2, GL_UNSIGNED_INT);

    // Bool vectors
    else if(type == GL_BOOL_VEC4)
        return std::make_tuple(4, GL_BOOL);
    else if(type == GL_BOOL_VEC3)
        return std::make_tuple(3, GL_BOOL);
    else if(type == GL_BOOL_VEC2)
        return std::make_tuple(2, GL_BOOL);
    else
        throw std::runtime_error("Unsupported GLSL type");
}