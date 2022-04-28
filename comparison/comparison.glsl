#pragma PARAM BENCHMARK;

layout(std430, binding = 3) buffer VertexBuffer {
    float vertices[9];
} vertexBuffer;

layout(std430, binding = 4) buffer ProgramBuffer {
    int color;
} programBuffer;

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

            programBuffer.color = 100;
            set_drawcommand(0, 3, 1, 0, 0, 0);
        }
    #endif
#endif

#ifdef PROGRAM_1
    #ifdef PROGRAM_1_COMPUTE_SHADER
        layout(local_size_x = 1) in;
    
        void main()
        {
            if(key_pressed(KEY_W) && programBuffer.color < 255)
                programBuffer.color += 1;
            if(key_pressed(KEY_S) && programBuffer.color > 0)
                programBuffer.color -= 1;
        }
    #endif
#endif

#ifdef PROGRAM_2
    #pragma PROGRAM_2_PARAM VBO VertexBuffer;

    #ifdef PROGRAM_2_VERTEX_SHADER
        layout (location = 0) in vec3 Position;

        void main()
        {
            gl_Position = vec4(Position.x, Position.y, Position.z, 1.0);
        }
    #endif

    #ifdef PROGRAM_2_FRAGMENT_SHADER
        out vec4 defaultOutput;

        void main() 
        {
            defaultOutput = vec4(programBuffer.color / 255.0, programBuffer.color / 255.0, programBuffer.color / 255.0, 1.0);
        }
    #endif
#endif