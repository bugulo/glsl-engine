#pragma PARAM HEIGHT 400;
#pragma PARAM WIDTH 400;
#pragma PARAM TITLE "Demo Application";

layout(std430, binding = 3) buffer VertexBuffer {
    float vertices[9];
} vertexBuffer;

layout(std430, binding = 4) buffer OtherBuffer {
    float rotation;
    float positionX;
    float positionY;
} otherBuffer;

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

            otherBuffer.rotation = 0;
            otherBuffer.positionX = 0;
            otherBuffer.positionY = 0;
        }
    #endif
#endif

// Program that updates the rotation
#ifdef PROGRAM_1
    #ifdef PROGRAM_1_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main()
        {
            otherBuffer.rotation += 20 * float(engineBuffer.deltaTime);

            if(key_pressed(KEY_W))
                otherBuffer.positionY += 5 * float(engineBuffer.deltaTime);
            if(key_pressed(KEY_S))
                otherBuffer.positionY -= 5 * float(engineBuffer.deltaTime);
            if(key_pressed(KEY_A))
                otherBuffer.positionX -= 5 * float(engineBuffer.deltaTime);
            if(key_pressed(KEY_D))
                otherBuffer.positionX += 5 * float(engineBuffer.deltaTime);
        }
    #endif
#endif

// Rendering program
#ifdef PROGRAM_2
    #pragma PROGRAM_2_PARAM VBO VertexBuffer;

    #ifdef PROGRAM_2_VERTEX_SHADER
        layout (location = 0) in vec3 Position;

        void main()
        {
            mat4 model = mat4(1.0f);
            model = translate(model, vec3(otherBuffer.positionX, otherBuffer.positionY, 0));
            model = rotate(model, radians(otherBuffer.rotation), vec3(0, 0, 1));

            gl_Position = model * vec4(Position, 1.0);
        }
    #endif

    #ifdef PROGRAM_2_FRAGMENT_SHADER
        out vec4 Color;

        void main() 
        {
            Color = vec4(1.0, 1.0, 1.0, 1.0);
        }
    #endif
#endif