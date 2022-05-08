#pragma PARAM HEIGHT 400;
#pragma PARAM WIDTH 400;
#pragma PARAM TITLE "Demo Application";

layout(std430, binding = 3) buffer VertexBuffer {
    float vertices[9];
} vertexBuffer;

// Initialization program
#ifdef PROGRAM_0
    #pragma PROGRAM_0_PARAM ONCE;

    #ifdef PROGRAM_0_COMPUTE_SHADER
        layout(local_size_x = 1) in;
    
        void main()
        {
            vertexBuffer.vertices = float[9](
                0.5f,  0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                -0.5f, -0.5f, 0.0f
            );

            set_drawcommand(0, 3, 1, 0, 0, 0);
        }
    #endif
#endif

// Rendering program
#ifdef PROGRAM_1
    #pragma PROGRAM_1_PARAM VBO VertexBuffer;

    #ifdef PROGRAM_1_VERTEX_SHADER
        layout (location = 0) in vec3 Position;

        void main()
        {
            gl_Position = vec4(Position, 1.0);
        }
    #endif

    #ifdef PROGRAM_1_FRAGMENT_SHADER
        out vec4 Color;

        void main() 
        {
            Color = vec4(1.0, 1.0, 1.0, 1.0);
        }
    #endif
#endif