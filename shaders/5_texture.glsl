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

// Program that updates the rotation and generates the texture
#ifdef PROGRAM_1
    #ifdef PROGRAM_1_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        layout(location = 0, rgba8) uniform image2D texture_100x100;

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

            // Generate random texture pattern
            for(int x = 0; x < 100; x++)
                for(int y = 0; y < 100; y++)
                {
                    float r = sin(int(engineBuffer.currentTime * 3) * x * y);
                    float g = cos(int(engineBuffer.currentTime * 3) * x);
                    float b = cos(int(engineBuffer.currentTime * 3) * y);
                    imageStore(texture_100x100, ivec2(x, y), vec4(r, g, b, 1.0));
                }
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
        in vec4 gl_FragCoord;

        uniform sampler2D texture_100x100;

        out vec4 Color;

        void main() 
        {
            Color = texture(texture_100x100, vec2(gl_FragCoord.x, gl_FragCoord.y) / 400);
        }
    #endif
#endif