#pragma PARAM HEIGHT 600;
#pragma PARAM WIDTH 800;
#pragma PARAM CURSOR_DISABLED;
#pragma PARAM TITLE "Example 3D camera";

layout(std430, binding = 3) buffer VBO {
    float v[24];
} vbo;

layout(std430, binding = 4) buffer EBO {
    uint v[36];
} ebo;

layout(std430, binding = 5) buffer Camera {
    vec3 cameraPos;
    vec3 cameraFront;
    vec3 cameraUp;

    float lastMouseX;
    float lastMouseY;

    float yaw;
    float pitch;
} camera;

#ifdef PROGRAM_0
    #pragma PROGRAM_0_PARAM ONCE;

    #ifdef PROGRAM_0_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main() {
            camera.cameraPos   = vec3(0.0f, 0.0f,  3.0f);
            camera.cameraFront = vec3(0.0f, 0.0f, -1.0f);
            camera.cameraUp    = vec3(0.0f, 1.0f,  0.0f);

            vbo.v = float[24]( 
                -1, -1, -1,
                 1, -1, -1,
                 1,  1, -1,
                -1,  1, -1,
                -1, -1,  1,
                 1, -1,  1,
                 1,  1,  1,
                -1,  1,  1
            );

            ebo.v = uint[36](
                0, 1, 3, 3, 1, 2,
                1, 5, 2, 2, 5, 6,
                5, 4, 6, 6, 4, 7,
                4, 0, 7, 7, 0, 3,
                3, 2, 7, 7, 2, 6,
                4, 5, 0, 0, 5, 1
            );

            set_drawcommand(0, 36, 1, 0, 0, 0);
            set_drawcommand(1, 36, 1, 0, 0, 0);

            camera.lastMouseX = float(engineBuffer.mouseX);
            camera.lastMouseY = float(engineBuffer.mouseY);
            camera.yaw = -90.0f;
            camera.pitch = 0.0f;
        }
    #endif
#endif

#ifdef PROGRAM_1
    #ifdef PROGRAM_1_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main()
        {
            // Calculate camera mouse look
            float xoffset = engineBuffer.mouseX - camera.lastMouseX;
            float yoffset = camera.lastMouseY - engineBuffer.mouseY; 
            camera.lastMouseX = engineBuffer.mouseX;
            camera.lastMouseY = engineBuffer.mouseY;

            float sensitivity = 2.0f * float(engineBuffer.deltaTime);
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            camera.yaw += xoffset;
            camera.pitch += yoffset;

            if(camera.pitch > 89.0f)
                camera.pitch = 89.0f;
            if(camera.pitch < -89.0f)
                camera.pitch = -89.0f;

            camera.cameraFront = normalize(vec3(
                cos(radians(camera.yaw)) * cos(radians(camera.pitch)),
                sin(radians(camera.pitch)),
                sin(radians(camera.yaw)) * cos(radians(camera.pitch))
            ));

            // Calculate camera movement
            float speed = 10.0 * float(engineBuffer.deltaTime);

            if(key_pressed(KEY_W))
                camera.cameraPos += speed * camera.cameraFront;
            if(key_pressed(KEY_S))
                camera.cameraPos -= speed * camera.cameraFront;
            if(key_pressed(KEY_A))
                camera.cameraPos -= normalize(cross(camera.cameraFront, camera.cameraUp)) * speed;
            if(key_pressed(KEY_D))
                camera.cameraPos += normalize(cross(camera.cameraFront, camera.cameraUp)) * speed;
        }
    #endif
#endif

#ifdef PROGRAM_2
    #pragma PROGRAM_2_PARAM VBO VBO;
    #pragma PROGRAM_2_PARAM EBO EBO;

    #ifdef PROGRAM_2_VERTEX_SHADER
        layout (location = 0) in vec3 aPos;

        void main()
        {
            mat4 model = mat4(1.0f);
            model = translate(model, vec3(float(gl_DrawID), float(gl_DrawID), float(gl_DrawID)));
            model = scale(model, vec3(0.5f, 0.5f, 0.5f));

            mat4 view = lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);

            mat4 projection = perspective(radians(45.0f), float(engineBuffer.width) / float(600), 0.1f, 100.0f);

            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    #endif

    #ifdef PROGRAM_2_FRAGMENT_SHADER
        out vec4 defaultOutput;

        void main()
        {
            defaultOutput = vec4(1.0f, 0.5f, 0.2f, 1.0f);
        }
    #endif
#endif