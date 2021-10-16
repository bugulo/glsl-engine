${engineShader}

void main() {
    const ivec3 position = ivec3(gl_GlobalInvocationID);

    if(key_pressed(KEY_B)) {
        imageStore(outputImage, position.xy, vec4(0.3f, 0.3f, 0.3f, 1.0f));
    } else if(key_pressed(KEY_A)) {
        imageStore(outputImage, position.xy, vec4(0.7f, 0.7f, 0.7f, 1.0f));
    } else {
        imageStore(outputImage, position.xy, vec4(0, 0, 0, 1.0f));
    }
}