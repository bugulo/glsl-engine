#pragma PARAM WIDTH 400;
#pragma PARAM HEIGHT 400;
#pragma PARAM TITLE "Demo Application";

layout(std430, binding = 3) buffer VertexBuffer {
    float data[9];
} vertexBuffer;

layout(std430, binding = 4) buffer OtherBuffer {
    int rotation;
} otherBuffer;

// Initialization program
#ifdef PROGRAM_0
    #pragma PROGRAM_0_PARAM ONCE;

    #ifdef PROGRAM_0_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main()
        {
            vertexBuffer.data = float[9](
                0.5, 0.5, 0,
                0.5, -0.5, 0,
                -0.5, -0.5, 0
            );

            set_drawcommand(0, 3, 1, 0, 0, 0);
            otherBuffer.rotation = 0;
        }
    #endif
#endif

#ifdef PROGRAM_2
    #ifdef PROGRAM_2_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        layout(location = 0, rgba8) uniform image2D texture_100x100;

        void main()
        {
            otherBuffer.rotation++;

            for(int x = 0; x < 100; x++)
                for(int y = 0; y < 100; y++)
                {
                    float r = sin(int(engineBuffer.currentTime * 60) * x * y);
                    float g = cos(int(engineBuffer.currentTime * 60) * x);
                    float b = cos(int(engineBuffer.currentTime * 60) * y);
                    imageStore(texture_100x100, ivec2(x, y), vec4(r, g, b, 1.0));
                }
        }
    #endif
#endif

#ifdef PROGRAM_1
    #pragma PROGRAM_1_PARAM VBO VertexBuffer;

    #ifdef PROGRAM_1_VERTEX_SHADER
        layout (location = 0) in vec3 Position;

        void main()
        {
            mat4 model = mat4(1.0);
            model = rotate(model, radians(otherBuffer.rotation), vec3(0, 0, 1));

            gl_Position = model * vec4(Position, 1.0);
        }
    #endif

    #ifdef PROGRAM_1_FRAGMENT_SHADER
        in vec4 gl_FragCoord;

        out vec4 Color;

        uniform sampler2D texture_100x100;

        void main()
        {
            if(key_pressed(KEY_SPACE))
                Color = texture(texture_100x100, vec2(gl_FragCoord.x, gl_FragCoord.y) / 500);
            else
                Color = vec4(1.0, 1.0, 1.0, 1.0);
        }
    #endif
#endif