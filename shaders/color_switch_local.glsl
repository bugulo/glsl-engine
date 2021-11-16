#ifdef PASS_0
    #ifdef PASS_0_COMPUTE_SHADER
        layout(local_size_x = 1) in;

        void main() {
            if(gl_NumWorkGroups.x == 1 && gl_LocalInvocationIndex == 0) {
                workGroupBuffer.x = 2;
                workGroupBuffer.y = 2;
                workGroupBuffer.z = 1;

                set_vertex(0, vec3( 1.0f,  1.0f, 0.0f));
                set_vertex(1, vec3( 1.0f, -1.0f, 0.0f));
                set_vertex(2, vec3(-1.0f, -1.0f, 0.0f));
                set_vertex(3, vec3(-1.0f,  1.0f, 0.0f));

                set_index(0, uvec3(0, 1, 3));
                set_index(1, uvec3(1, 2, 3));

                set_drawcommand(0, 6, 0, 0, 0);
            }
        }
    #endif
#endif

#ifdef PASS_1
    #ifdef PASS_1_COMPUTE_SHADER
        layout(local_size_x = 4) in;

        shared uint done;

        void main() {
            if(gl_LocalInvocationIndex == 0) {
                done = 0;
            }

            barrier();

            while(done < 256 * 256)
            {
                uint previous = atomicAdd(done, 1);

                // Calculate image position
                const uint x = previous % 256;
                const uint y = previous / 256;
                const ivec3 position = ivec3(gl_WorkGroupID * uvec3(256, 256, 1)) + ivec3(x, y, 0);

                if(key_pressed(KEY_B)) {
                    imageStore(outputImage, position.xy, vec4(0.3f, 0.3f, 0.3f, 1.0f));
                } else if(key_pressed(KEY_A)) {
                    imageStore(outputImage, position.xy, vec4(0.7f, 0.7f, 0.7f, 1.0f));
                } else {
                    imageStore(outputImage, position.xy, vec4(0, 0, 0, 1.0f));
                }
            }
        }
    #endif
#endif

#ifdef PASS_2
    #ifdef PASS_2_VERTEX_SHADER
        layout (location = 0) in vec3 aPos;

        void main()
        {
            gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
        }
    #endif

    #ifdef PASS_2_FRAGMENT_SHADER
        uniform sampler2D inputImage;

        layout(location = 0) out vec4 fragColor;

        void main() {
            vec2 position = vec2(gl_FragCoord.x, gl_FragCoord.y);
            fragColor = texture(inputImage, position);
        }
    #endif
#endif