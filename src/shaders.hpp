const char *engineShaderSource = R"(
    #version 440 core

    #define KEY_A 65
    #define KEY_B 66

    #define STATE_PRESSED 1

    layout(rgba8) writeonly uniform image2D outputImage;

    layout(local_size_x = 32, local_size_y = 32) in;

    layout(std430, binding = 0) buffer EngineBuffer {
        int keyState[348];
    };

    bool key_pressed(uint key) {
        return keyState[key] == 1;
    }
)";

const char *vertexShaderSource = R"(
    #version 440 core

    out vec2 position;

    void main() {
        position = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
        gl_Position = vec4(2.0f * position - 1.0f, 0.0f, 1.0f);
    }
)";

const char *fragmentShaderSource = R"(
    #version 440 core

    uniform sampler2D inputImage;

    in vec2 position;

    layout(location = 0) out vec4 fragColor;

    void main() {
        fragColor = texture(inputImage, position);
    }
)";