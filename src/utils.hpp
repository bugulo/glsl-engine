#ifndef UTILS_H
#define UTILS_H

#include <tuple>

#include <GL/glew.h>

class Utils
{
public:
    /*!
     * @brief Get size of GLSL type in bytes
     * @param type GLSL type
     * @return GLSL type size in bytes
     */
    static GLsizei getTypeSize(GLenum type);

    /*!
     * @brief Get memory format of GLSL type
     *        for example vec3 consists of 3 x GL_FLOAT variables
     * @param type GLSL type
     * @return Tuple where first item represents number of variables of type from second item
     */
    static std::tuple<GLint, GLenum> getTypeFormat(GLenum type);
};

#endif