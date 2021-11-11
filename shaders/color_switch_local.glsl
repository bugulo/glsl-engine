#ifdef PASS_0
    #ifdef PASS_0_COMPUTE_SHADER
        layout(local_size_x = 4) in;

        shared uint done;

        void main() {
            if(gl_LocalInvocationIndex == 0 && gl_NumWorkGroups.x == 1) {
                workGroupBuffer.x = 2;
                workGroupBuffer.y = 2;
                workGroupBuffer.z = 1;
                return;
            }

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

#ifdef PASS_1
    #ifdef PASS_1_VERTEX_SHADER
        out vec2 position;

        void main() {
            position = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
            gl_Position = vec4(2.0f * position - 1.0f, 0.0f, 1.0f);
        }
    #endif

    #ifdef PASS_1_FRAGMENT_SHADER
        uniform sampler2D inputImage;

        in vec2 position;

        layout(location = 0) out vec4 fragColor;

        void main() {
            fragColor = texture(inputImage, position);
        }
    #endif
#endif