const char *engineShaderSource = R"(
#version 440 core

#extension GL_ARB_shader_ballot : enable

#define KEY_A 65
#define KEY_B 66

#define STATE_PRESSED 1

struct DrawCommand {
    uint count;
    uint instanceCount;
    uint firstIndex;
    uint baseVertex;
    uint baseInstance;
};

layout(location = 0, rgba8) uniform image2D outputImage;

layout(std430, binding = 0) volatile buffer InputBuffer {
    int keys[348];
} inputBuffer;

layout(std430, binding = 1) buffer WorkGroupBuffer {
    uint x;
    uint y;
    uint z; 
} workGroupBuffer;

layout(std140, binding = 2) writeonly buffer DrawCommandBuffer {
    DrawCommand commands[];
} drawCommandBuffer;

layout(std430, binding = 3) buffer VertexBuffer {
    float vertices[100];
} vertexBuffer;

layout(std430, binding = 4) buffer ElementBuffer {
    uint indices[100];
} elementBuffer;

void set_drawcommand(uint offset, uint count, uint firstIndex, uint baseVertex, uint baseInstance)
{
    drawCommandBuffer.commands[offset].count = count;
    drawCommandBuffer.commands[offset].instanceCount = 1;
    drawCommandBuffer.commands[offset].firstIndex = firstIndex;
    drawCommandBuffer.commands[offset].baseVertex = baseVertex;
    drawCommandBuffer.commands[offset].baseInstance = baseInstance;
}

void set_vertex(uint offset, vec3 position) {
    vertexBuffer.vertices[offset * 3]       = position.x;
    vertexBuffer.vertices[offset * 3 + 1]   = position.y;
    vertexBuffer.vertices[offset * 3 + 2]   = position.z;
}

void set_index(uint offset, uvec3 position) {
    elementBuffer.indices[offset * 3]       = position.x;
    elementBuffer.indices[offset * 3 + 1]   = position.y;
    elementBuffer.indices[offset * 3 + 2]   = position.z;
}

bool key_pressed(uint key) {
    return inputBuffer.keys[key] == 1;
}

)";