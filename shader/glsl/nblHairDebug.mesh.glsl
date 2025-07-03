#version 460
#extension GL_EXT_mesh_shader : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#ifdef DEBUG
    #extension GL_EXT_debug_printf : enable
#endif

#extension GL_GOOGLE_include_directive : enable
#include "inc/hairCommon.glsl"

layout (local_size_x = WORKGROUP_SIZE) in;
layout (triangles, max_vertices = 128, max_primitives = 64) out;

layout (push_constant) uniform HairConstants {
    mat4     model;
    vec4     hair_diffuse;
    vec4     hair_specular;
    int      vertexCount;
    int      strandCount;
    int      renderingMode;
    int      _pad0;
    uint64_t vertex_address;
    uint64_t sdesc_address;
} hair_constants;

layout (buffer_reference, scalar) buffer Vertices { HairVertex vertices[]; };

layout (buffer_reference, scalar) buffer StrandDescriptions { StrandDescription descriptions[]; };

layout (set = 0, binding = 0) uniform CameraData {
    mat4  view;
    mat4  proj;
    mat4  view_inverse;
    mat4  proj_inverse;
    vec4  eye;
    float near_plane;
    float far_plane;
} camera;

// Input --------------------------------
taskPayloadSharedEXT Task IN;

uint workGroupID = gl_WorkGroupID.x;
uint laneID      = gl_LocalInvocationID.x;

// Output -------------------------------
layout (location = 0) out MeshDataDebug m_out[];

// Functions ----------------------------
StrandDescription getStrandDescription(uint id) {
    StrandDescriptions sds = StrandDescriptions(hair_constants.sdesc_address);
    return sds.descriptions[id];
}

uint getStrandletVertexCount(uint strandletID, uint totalVertexCount) {
    return min(totalVertexCount - (WORKGROUP_SIZE * strandletID), WORKGROUP_SIZE);
}

HairVertex[4] buildQuad(uint offset) {
    Vertices v = Vertices(hair_constants.vertex_address);
    HairVertex result[4];

    result[0] = v.vertices[offset + 0];
    result[1] = v.vertices[offset + 0];
    result[2] = v.vertices[offset + 1];
    result[3] = v.vertices[offset + 1];

    float t = 0.15;
    result[1].position += vec4(t, 0, 0, 0);
    result[2].position += vec4(t, 0, 0, 0);

    return result;
}

void main()
{
    uint deltaID = 0;
    uint k = 0;
    for (uint i = 0; i < WORKGROUP_SIZE; i++) {
        if (workGroupID < uint(IN.deltaID[i])) break;
        deltaID = uint(IN.deltaID[i]);
        k = i;
    }

    // Current [Strand] information
    uint              current_strandID    = IN.baseID + k;
    StrandDescription strand_description  = getStrandDescription(current_strandID);
    uint              strand_vertex_count = strand_description.vertex_count;
    uint              base_vertex_offset  = strand_description.vertex_offset;

    // Current [Strandlet] information
    uint strandletID        = workGroupID - deltaID;
    uint strandlet_vertices = getStrandletVertexCount(strandletID, strand_vertex_count);

    // Calculate output parameters
    uint n_quads = strandlet_vertices - 1;
    uint n_tri   = n_quads * 2;
    uint n_vtx   = n_quads * 4;

    // Do no work if current lane exceeds quad count
    if (laneID > n_quads) return;

    SetMeshOutputsEXT(n_vtx, n_tri);

    // Calculate global offset into the vertex buffer
    uint vertex_buffer_offset = base_vertex_offset + (strandletID * WORKGROUP_SIZE) + laneID - strandletID;

    const mat4 M  = hair_constants.model;
    const mat4 VP = camera.proj * camera.view;

    const HairVertex quad[4] = buildQuad(vertex_buffer_offset);

    vec4 tangent = vec4(quad[3].position.xyz - quad[0].position.xyz, 0.0);
    vec4 world_tangent = normalize(vec4((M * tangent).xyz, 0.0));

    const uint vtx_out_offset = laneID * 4;
    const uint tri_out_offset = laneID * 2;

    vec4 color = getColor(strandletID + current_strandID + laneID);
    if (hair_constants.renderingMode == 1) {
        color = getColor(strandletID + current_strandID + laneID);
    }
    if (hair_constants.renderingMode == 2) {
        color = getColor(current_strandID);
    }
    if (hair_constants.renderingMode == 3) {
        color = getColor(strandletID);
    }

    for (uint i = 0; i < 4; i++) {
        vec4 world_position = hair_constants.model * quad[i].position;

        gl_MeshVerticesEXT[vtx_out_offset + i].gl_Position = VP * world_position;

        m_out[vtx_out_offset + i].world_position = world_position;
        m_out[vtx_out_offset + i].world_tangent  = world_tangent;
        m_out[vtx_out_offset + i].color          = color;
    }

    gl_PrimitiveTriangleIndicesEXT[tri_out_offset + 0] = uvec3(2, 1, 0) + vtx_out_offset;
    gl_PrimitiveTriangleIndicesEXT[tri_out_offset + 1] = uvec3(3, 2, 0) + vtx_out_offset;
}
