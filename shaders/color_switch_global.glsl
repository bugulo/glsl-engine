${engineShader}

layout(local_size_x = 16) in;

void main() {
    if(gl_LocalInvocationIndex == 0 && gl_NumWorkGroups.x == 1) {
        workGroupBuffer.x = 2;
        workGroupBuffer.y = 2;
        workGroupBuffer.z = 1;
        return;
    }

    if(gl_GlobalInvocationID.x == 0) {
        inputBuffer.keys[0] = 0;
    }

    barrier();

    while(inputBuffer.keys[0] < 512 * 512)
    {
        uint previous = atomicAdd(inputBuffer.keys[0], 1);

        // Calculate image position
        const uint x = previous % 512;
        const uint y = previous / 512;
        const ivec3 position = ivec3(gl_GlobalInvocationID) + ivec3(x, y, 0);

        if(key_pressed(KEY_B)) {
            imageStore(outputImage, position.xy, vec4(0.3f, 0.3f, 0.3f, 1.0f));
        } else if(key_pressed(KEY_A)) {
            imageStore(outputImage, position.xy, vec4(0.7f, 0.7f, 0.7f, 1.0f));
        } else {
            imageStore(outputImage, position.xy, vec4(0, 0, 0, 1.0f));
        }
    }
}