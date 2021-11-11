#ifdef PASS_0
    #ifdef PASS_0_COMPUTE_SHADER
        layout(local_size_x = 16) in;

        void main() {
            if(gl_LocalInvocationIndex == 0 && gl_NumWorkGroups.x == 1) {
                workGroupBuffer.x = 4;
                workGroupBuffer.y = 1;
                workGroupBuffer.z = 1;
                return;
            }

            barrier();

            do {
                uint previous;
                if(gl_LocalInvocationID.x == 0)
                    previous = atomicAdd(inputBuffer.keys[0], 32);

                previous = readFirstInvocationARB(previous);

                if(previous >= 512 * 512)
                    return;

                // Calculate image position
                const uint x = previous % 512;
                const uint y = previous / 512;
                const ivec3 position = ivec3(gl_LocalInvocationID) + ivec3(x, y, 0);

                if(key_pressed(KEY_B)) {
                    imageStore(outputImage, position.xy, vec4(0.3f, 0.3f, 0.3f, 1.0f));
                } else if(key_pressedb(KEY_A)) {
                    imageStore(outputImage, position.xy, vec4(0.7f, 0.7f, 0.7f, 1.0f));
                } else {
                    imageStore(outputImage, position.xy, vec4(0, 0, 0, 1.0f));
                }
            } while(true);

            imageStore(outputImage, ivec2(2, 2), vec4(0, 255, 0, 1.0f));
        }
    #endif
#endif