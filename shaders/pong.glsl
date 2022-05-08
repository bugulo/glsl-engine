#pragma PARAM HEIGHT 600;
#pragma PARAM WIDTH 800;
#pragma PARAM TITLE "Pong";

#pragma PARAM ENABLE_DEPTH_TEST;

layout(std430, binding = 3) buffer VBO {
    float v[216];
} vbo;

struct Transform
{
    vec3 position;
    vec3 scale;
};

mat4 getTransform(Transform transform)
{
    mat4 mat = mat4(1.0f);
    mat = translate(mat, transform.position);
    mat = scale(mat, transform.scale);
    return mat;
}

bool intersect(Transform first, Transform second) {
    vec3 aMin = vec3(99999999, 99999999, 99999999); vec3 aMax = vec3(-99999999, -99999999, -99999999);
    vec3 bMin = vec3(99999999, 99999999, 99999999); vec3 bMax = vec3(-99999999, -99999999, -99999999);

    for(int i = 0; i < 216; i += 6)
    {
        vec3 position = vec3(vbo.v[i], vbo.v[i + 1], vbo.v[i + 2]);
        vec4 a = getTransform(first) * vec4(position, 1);
        vec4 b = getTransform(second) * vec4(position, 1);

        if(a.x < aMin.x)
            aMin.x = a.x;
        if(a.y < aMin.y)
            aMin.y = a.y;
        if(a.z < aMin.z)
            aMin.z = a.z;

        if(a.x > aMax.x)
            aMax.x = a.x;
        if(a.y > aMax.y)
            aMax.y = a.y;
        if(a.z > aMax.z)
            aMax.z = a.z;

        if(b.x < bMin.x)
            bMin.x = b.x;
        if(b.y < bMin.y)
            bMin.y = b.y;
        if(b.z < bMin.z)
            bMin.z = b.z;

        if(b.x > bMax.x)
            bMax.x = b.x;
        if(b.y > bMax.y)
            bMax.y = b.y;
        if(b.z > bMax.z)
            bMax.z = b.z; 
    }

    return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
            (aMin.y <= bMax.y && aMax.y >= bMin.y) &&
            (aMin.z <= bMax.z && aMax.z >= bMin.z);
}

layout(std430, binding = 5) buffer Camera {
    vec3 cameraPos;
    vec3 cameraFront;
    vec3 cameraUp;
} camera;

layout(std430, binding = 6) buffer Game {
    vec3 ballVelocity;
} game;

layout(std430, binding = 7) buffer Objects {
    Transform plane;
    Transform player1;
    Transform player2;
    Transform topWall;
    Transform bottomWall;
    Transform leftWall;
    Transform rightWall;
    Transform ball;
} objects;

#ifdef PROGRAM_0
    #pragma PROGRAM_0_PARAM ONCE;

    #ifdef PROGRAM_0_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main() {
            camera.cameraPos   = vec3(0.0, -4.0,  11.0);
            camera.cameraFront = vec3(0.0, 0.3, -1.0);
            camera.cameraUp    = vec3(0.0, 1.0,  0.0);

            vbo.v = float[216]( 
                -0.5, -0.5, -0.5,  0.0,  0.0, -1.0,
                 0.5, -0.5, -0.5,  0.0,  0.0, -1.0,
                 0.5,  0.5, -0.5,  0.0,  0.0, -1.0,
                 0.5,  0.5, -0.5,  0.0,  0.0, -1.0,
                -0.5,  0.5, -0.5,  0.0,  0.0, -1.0,
                -0.5, -0.5, -0.5,  0.0,  0.0, -1.0,

                -0.5, -0.5,  0.5,  0.0,  0.0,  1.0,
                 0.5, -0.5,  0.5,  0.0,  0.0,  1.0,
                 0.5,  0.5,  0.5,  0.0,  0.0,  1.0,
                 0.5,  0.5,  0.5,  0.0,  0.0,  1.0,
                -0.5,  0.5,  0.5,  0.0,  0.0,  1.0,
                -0.5, -0.5,  0.5,  0.0,  0.0,  1.0,

                -0.5,  0.5,  0.5, -1.0,  0.0,  0.0,
                -0.5,  0.5, -0.5, -1.0,  0.0,  0.0,
                -0.5, -0.5, -0.5, -1.0,  0.0,  0.0,
                -0.5, -0.5, -0.5, -1.0,  0.0,  0.0,
                -0.5, -0.5,  0.5, -1.0,  0.0,  0.0,
                -0.5,  0.5,  0.5, -1.0,  0.0,  0.0,

                 0.5,  0.5,  0.5,  1.0,  0.0,  0.0,
                 0.5,  0.5, -0.5,  1.0,  0.0,  0.0,
                 0.5, -0.5, -0.5,  1.0,  0.0,  0.0,
                 0.5, -0.5, -0.5,  1.0,  0.0,  0.0,
                 0.5, -0.5,  0.5,  1.0,  0.0,  0.0,
                 0.5,  0.5,  0.5,  1.0,  0.0,  0.0,

                -0.5, -0.5, -0.5,  0.0, -1.0,  0.0,
                 0.5, -0.5, -0.5,  0.0, -1.0,  0.0,
                 0.5, -0.5,  0.5,  0.0, -1.0,  0.0,
                 0.5, -0.5,  0.5,  0.0, -1.0,  0.0,
                -0.5, -0.5,  0.5,  0.0, -1.0,  0.0,
                -0.5, -0.5, -0.5,  0.0, -1.0,  0.0,

                -0.5,  0.5, -0.5,  0.0,  1.0,  0.0,
                 0.5,  0.5, -0.5,  0.0,  1.0,  0.0,
                 0.5,  0.5,  0.5,  0.0,  1.0,  0.0,
                 0.5,  0.5,  0.5,  0.0,  1.0,  0.0,
                -0.5,  0.5,  0.5,  0.0,  1.0,  0.0,
                -0.5,  0.5, -0.5,  0.0,  1.0,  0.0
            );

            set_drawcommand(0, 36, 8, 0, 0, 0);

            game.ballVelocity = vec3(cos(radians(randInt(0, 360))), sin(radians(randInt(0, 360))), 0);

            objects.player1.position = vec3(-9, 0, 0);
            objects.player1.scale = vec3(0.2, 2, 0.1);

            objects.player2.position = vec3(9, 0, 0);
            objects.player2.scale = vec3(0.2, 2, 0.1);

            objects.ball.position = vec3(0, 0, 0);
            objects.ball.scale = vec3(0.5, 0.5, 0.5);

            objects.topWall.position = vec3(0, 5, 0);
            objects.topWall.scale = vec3(20.5, 0.5, 0.5);

            objects.bottomWall.position = vec3(0, -5, 0);
            objects.bottomWall.scale = vec3(20.5, 0.5, 0.5);

            objects.leftWall.position = vec3(-10, 0, 0);
            objects.leftWall.scale = vec3(0.5, 9.5, 0.5);

            objects.rightWall.position = vec3(10, 0, 0);
            objects.rightWall.scale = vec3(0.5, 9.5, 0.5);

            objects.plane.position = vec3(0, 0, -0.5);
            objects.plane.scale = vec3(20, 10, 0.01);
        }
    #endif
#endif

#ifdef PROGRAM_1
    #ifdef PROGRAM_1_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main()
        {
            float playerSpeed = 10.0 * float(engineBuffer.deltaTime);
            float ballSpeed = 10.0 * float(engineBuffer.deltaTime);

            // Player1 movement
            if(key_pressed(KEY_W) && objects.player1.position.y < 4)
                objects.player1.position.y += 2 * playerSpeed;
            else if(key_pressed(KEY_S) && objects.player1.position.y > -4)
                objects.player1.position.y -= 2 * playerSpeed;

            // Player2 movement
            if(key_pressed(KEY_UP) && objects.player2.position.y < 4)
                objects.player2.position.y += 2 * playerSpeed;
            else if(key_pressed(KEY_DOWN) && objects.player2.position.y > -4)
                objects.player2.position.y -= 2 * playerSpeed;

            // Ball movement
            objects.ball.position += game.ballVelocity * ballSpeed;

            if(intersect(objects.player1, objects.ball))
                game.ballVelocity.x *= -1;
            if(intersect(objects.player2, objects.ball))
                game.ballVelocity.x *= -1;
            if(intersect(objects.topWall, objects.ball) || intersect(objects.bottomWall, objects.ball))
                game.ballVelocity.y *= -1;
            // Reset game
            if(intersect(objects.leftWall, objects.ball) || intersect(objects.rightWall, objects.ball))
            {
                objects.ball.position = vec3(0, 0, 0);
                game.ballVelocity = vec3(cos(radians(randInt(0, 360))), sin(radians(randInt(0, 360))), 0);
            }
        }
    #endif
#endif

#ifdef PROGRAM_2
    #pragma PROGRAM_2_PARAM VBO VBO;

    #ifdef PROGRAM_2_VERTEX_SHADER
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;

        out vec3 Normal;
        out vec3 FragPos;
        out int Instance;

        void main()
        {
            // Model space matrix
            mat4 model = mat4(1.0f);
            // Camera view matrix
            mat4 view = lookAt(camera.cameraPos, camera.cameraPos + camera.cameraFront, camera.cameraUp);
            // Projection matrix
            mat4 projection = perspective(radians(80), float(engineBuffer.width) / float(engineBuffer.height), 0.1, 100.0);

            if(gl_InstanceID == 0)
                model = getTransform(objects.player1);
            else if(gl_InstanceID == 1)
                model = getTransform(objects.player2);
            else if(gl_InstanceID == 2)
                model = getTransform(objects.ball);
            else if(gl_InstanceID == 3)
                model = getTransform(objects.topWall);
            else if(gl_InstanceID == 4)
                model = getTransform(objects.bottomWall);
            else if(gl_InstanceID == 5)
                model = getTransform(objects.leftWall);
            else if(gl_InstanceID == 6)
                model = getTransform(objects.rightWall);
            else if(gl_InstanceID == 7)
                model = getTransform(objects.plane);

            Normal = mat3(transpose(inverse(model))) * aNormal; 
            FragPos = vec3(model * vec4(aPos, 1.0));
            Instance = gl_InstanceID;
            
            gl_Position = projection * view * model * vec4(aPos, 1.0);

            // Weird hack
            if(aNormal.x == 5)
                gl_Position = vec4(gl_Position.x, gl_Position.y, gl_Position.z, gl_Position.w);
        }
    #endif

    #ifdef PROGRAM_2_FRAGMENT_SHADER
        out vec4 defaultOutput;

        in vec3 Normal;
        in vec3 FragPos;
        flat in int Instance;

        void main()
        {
            // Scene lighting calculation inspired by:
            /*  @see https://learnopengl.com/Lighting/Basic-Lighting
                -- LearnOpenGL - Basic Lighting
                -- author - Joey de Vries
                -- cited: 2022-04-01 */

            vec3 lightPos = vec3(0, 0, 7);
            vec3 objectColor = vec3(1.0, 1.0, 1.0);
            vec3 lightColor = vec3(1, 1, 1);

            if(Instance == 0)
                objectColor = vec3(1, 0, 0);
            else if(Instance == 1)
                objectColor = vec3(1, 0, 0);
            else if(Instance == 2)
                objectColor = vec3(1, 0, 0);
            else if(Instance == 3)
                objectColor = vec3(0.9, 0.9, 0.9);
            else if(Instance == 4)
                objectColor = vec3(0.9, 0.9, 0.9);
            else if(Instance == 5)
                objectColor = vec3(0.3, 0.3, 0.3);
            else if(Instance == 6)
                objectColor = vec3(0.3, 0.3, 0.3);
            else if(Instance == 7)
                objectColor = vec3(0.4, 0.6, 1);

            // ambient
            float ambientStrength = 0.1;
            vec3 ambient = ambientStrength * lightColor;
            
            // diffuse 
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            
            // specular
            float specularStrength = 0.5;
            vec3 viewDir = normalize(camera.cameraPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;  
                
            vec3 result = (ambient + diffuse + specular) * objectColor;
            defaultOutput = vec4(result, 1.0);
        }
    #endif
#endif