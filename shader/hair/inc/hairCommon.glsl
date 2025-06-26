#extension GL_EXT_shader_explicit_arithmetic_types_int8 : require

#ifndef WORKGROUP_SIZE
    #define WORKGROUP_SIZE 32
#endif

// Task Shader Payload
struct Task {
    uint    baseID;
    uint8_t deltaID[WORKGROUP_SIZE - 1];
};

// Mesh Shader Payload
struct MeshData {
    vec4 wPosition;
    vec4 wTangent;
};

// Hair Vertex
struct HairVertex {
    vec4 position;
};

struct StrandDescription {
    int strandId;
    int vertexCount;
    int strandletCount;
    int vertexOffset;
};
