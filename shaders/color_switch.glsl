layout(std430, binding = 5) volatile buffer MyBuffer {
    uint counter;
} myBuffer;

layout(std430, binding = 6) buffer Myvbo {
    float vertices[100];
} myvbo;

layout(std430, binding = 7) buffer Myebo {
    uint indices[100];
} myebo;

void set_vertex(uint offset, vec3 position) {
    myvbo.vertices[offset * 3]       = position.x;
    myvbo.vertices[offset * 3 + 1]   = position.y;
    myvbo.vertices[offset * 3 + 2]   = position.z;
}

void set_index(uint offset, uvec3 position) {
    myebo.indices[offset * 3]       = position.x;
    myebo.indices[offset * 3 + 1]   = position.y;
    myebo.indices[offset * 3 + 2]   = position.z;
}

#ifdef PASS_0
    #pragma PASS_0_PARAM ONCE;

    #ifdef PASS_0_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main() {
            //if(gl_NumWorkGroups.x == 1 && gl_LocalInvocationIndex == 0) {
                workGroupBuffer.x = 4;
                workGroupBuffer.y = 1;
                workGroupBuffer.z = 1;

                set_vertex(0, vec3( 1.0f,  1.0f, 0.0f));
                set_vertex(1, vec3( 1.0f, -1.0f, 0.0f));
                set_vertex(2, vec3(-1.0f, -1.0f, 0.0f));
                set_vertex(3, vec3(-1.0f,  1.0f, 0.0f));

                set_index(0, uvec3(0, 1, 3));
                set_index(1, uvec3(1, 2, 3));

                set_drawcommand(0, 6, 1, 0, 0, 0);
            //}
        }
    #endif
#endif

#ifdef PASS_1
    #ifdef PASS_1_COMPUTE_SHADER
        layout(local_size_x = 16) in;

        layout(location = 0, rgba8) uniform image2D testTexture1_500x500;
        layout(location = 1, rgba8) uniform image2D testTexture2_500x500;

        void main() {
            if(gl_LocalInvocationID.x == 0)
                myBuffer.counter = 0;

            barrier();
            
            do
            {
                uint previous;
                if(gl_LocalInvocationID.x == 0)
                    previous = atomicAdd(myBuffer.counter, 16);

                previous = readFirstInvocationARB(previous);

                if(previous >= 512 * 512)
                    return;

                // Calculate image position
                const uint x = previous % 512;
                const uint y = previous / 512;
                const ivec3 position = ivec3(gl_LocalInvocationID) + ivec3(x, y, 0);

                if(mouse_pressed(MOUSE_BUTTON_LEFT)) {
                    imageStore(testTexture1_500x500, position.xy, vec4(0.3f, 0.3f, 0.3f, 1.0f));
                    imageStore(testTexture2_500x500, position.xy, vec4(0.7f, 0.7f, 0.7f, 1.0f));
                } else if(mouse_pressed(MOUSE_BUTTON_RIGHT)) {
                    imageStore(testTexture1_500x500, position.xy, vec4(0.7f, 0.7f, 0.7f, 1.0f));
                    imageStore(testTexture2_500x500, position.xy, vec4(0.3f, 0.3f, 0.3f, 1.0f));
                } else {
                    imageStore(testTexture1_500x500, position.xy, vec4(0, 0, 0, 1.0f));
                    imageStore(testTexture2_500x500, position.xy, vec4(1, 1, 1, 1.0f));
                }
            } while(true);
        }
    #endif
#endif

#ifdef PASS_2
    #pragma PASS_2_PARAM VBO Myvbo;
    #pragma PASS_2_PARAM EBO Myebo;

    #ifdef PASS_2_VERTEX_SHADER
        layout (location = 0) in vec3 aPos;

        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    #endif

    #ifdef PASS_2_FRAGMENT_SHADER
        uniform sampler2D testTexture1_500x500;
        uniform sampler2D testTexture2_500x500;

        out vec4 testTexture3_500x500;

        void main() {
            vec2 position = vec2(gl_FragCoord.x, gl_FragCoord.y);
            if(position.x > position.y)
                testTexture3_500x500 = texture(testTexture1_500x500, position);
            else
                testTexture3_500x500 = texture(testTexture2_500x500, position);
        }
    #endif
#endif

#ifdef PASS_3
    #pragma PASS_3_PARAM VBO Myvbo;
    #pragma PASS_3_PARAM EBO Myebo;

    #ifdef PASS_3_VERTEX_SHADER
        layout (location = 0) in vec3 aPos;

        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    #endif

    #ifdef PASS_3_FRAGMENT_SHADER
        uniform sampler2D testTexture3_500x500;

        out vec4 defaultOutput;

        void main() {
            vec2 position = vec2(gl_FragCoord.x, gl_FragCoord.y) / engineBuffer.mouseX;
            defaultOutput = texture(testTexture3_500x500, position);
        }
    #endif
#endif