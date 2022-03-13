layout(std430, binding = 3) buffer VBOplayer {
    float v[12];
} vbo_player;

layout(std430, binding = 4) buffer EBOplayer {
    uint v[6];
} ebo_player;

mat4 ortho(float left, float right, float bottom, float top, float near, float far) {
    return mat4(
        vec4(2.0/(right-left), 0.0, 0.0, 0.0),
        vec4(0.0, 2.0/(top-bottom), 0.0, 0.0),
        vec4(0.0,0.0,-2.0/(far-near), 0.0),
        vec4(-(right+left)/(right-left),-(top+bottom)/(top-bottom),-(far+near)/(far-near),1.0)
    );
}

mat4 perspective(float fov, float aspect, float near, float far) {
    return mat4(
        vec4(1.0/(aspect * tan(fov/2.0)), 0, 0, 0),
        vec4(0,1.0/tan(fov/2.0), 0, 0),
        vec4(0,0,-(far+near)/(far-near), -1),
        vec4(0,0, 2*(far*near)/(far-near), 0)
    );
}

#ifdef PASS_0
    #pragma PASS_0_PARAM ONCE;

    #ifdef PASS_0_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main() {
            vbo_player.v = float[12]( 
                0.5f,  0.5f, 10.0f,  // top right
                0.5f, -0.5f, 10.0f,  // bottom right
                -0.5f, -0.5f, 10.0f,  // bottom left
                -0.5f,  0.5f, 10.0f   // top left 
            );

            ebo_player.v = uint[6](
                0, 1, 3,   // first triangle
                1, 2, 3    // second triangle
            );

            set_drawcommand(0, 6, 0, 0, 0);
        }
    #endif
#endif

#ifdef PASS_1
    #pragma PASS_1_PARAM VBO VBOplayer;
    #pragma PASS_1_PARAM EBO EBOplayer;

    #ifdef PASS_1_VERTEX_SHADER
        layout (location = 0) in vec3 aPos;

        void main()
        {
            mat4 trans = mat4(1.0f);
            mat4 scale = mat4(
                vec4(10.0, 0, 0, 0),
                vec4(0, 10.0, 0, 0),
                vec4(0, 0, 1.0, 0),
                vec4(0, 0, 0, 1)
            );
            /*mat4 rotate = mat4(
                vec4(cos(float(engineBuffer.currentTime)), -sin(float(engineBuffer.currentTime)), 0, 0),
                vec4(sin(float(engineBuffer.currentTime)), cos(float(engineBuffer.currentTime)), 0, 0),
                vec4(0, 0, 1, 0),
                vec4(0, 0, 0, 1)
            );*/
            mat4 rotate = mat4(
                vec4(1, 0, 0, 0),
                vec4(0, cos(float(engineBuffer.currentTime)), sin(float(engineBuffer.currentTime)), 0),
                vec4(0, -sin(float(engineBuffer.currentTime)), cos(float(engineBuffer.currentTime)), 0),
                vec4(0, 0, 0, 1)
            );
            mat4 model = mat4(1.0f);
            mat4 projection = perspective(radians(90.0f), 512.0f/512.0f, 0.1f, 100.0f);
            gl_Position = vec4(aPos, 1.0) * projection;
        }
    #endif

    #ifdef PASS_1_FRAGMENT_SHADER
        out vec4 defaultOutput;

        void main()
        {
            defaultOutput = vec4(1.0f, 0.5f, 0.2f, 1.0f);
        }
    #endif
#endif