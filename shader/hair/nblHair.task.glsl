#version 460

#extension GL_EXT_mesh_shader : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_EXT_scalar_block_layout : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#ifdef DEBUG
    #extension GL_EXT_debug_printf : enable
#endif

#extension GL_GOOGLE_include_directive : enable
#include "inc/hairCommon.glsl"

layout (local_size_x = WORKGROUP_SIZE) in;

layout (push_constant) uniform HairConstants {
    mat4     model;
    vec4     hair_diffuse;
    vec4     hair_specular;
    int      vertexCount;
    int      strandCount;
    int      _pad0;
    int      _pad1;
    uint64_t vertex_address;
    uint64_t sdesc_address;
} hair_constants;

layout (buffer_reference, scalar) buffer StrandDescriptions {
    StrandDesc descriptions[];
};

// Input --------------------------------
uint baseID = gl_WorkGroupID.x * WORKGROUP_SIZE;
uint laneID = gl_LocalInvocationID.x;

// Output -------------------------------
taskPayloadSharedEXT Task OUT;

// Functions ----------------------------
int getStrandCount() { return hair_constants.strandCount; }

StrandDesc getStrandDescription(uint id) {
    StrandDescriptions sds = StrandDescriptions(hair_constants.sdesc_address);
    return sds.descriptions[id];
}

void main()
{
    uint l_strandID = laneID;                       // Relative to Workgroup (Local) Strand ID
    uint g_strandID = baseID + l_strandID;          // Global Strand ID

    if (g_strandID >= getStrandCount()) {
        return;
    }

    StrandDesc strand_description = getStrandDescription(g_strandID);
    int        strandlet_count    = strand_description.strandlet_count;
    uint       strand_wg_offset   = subgroupExclusiveAdd(strandlet_count);

    if (laneID != 0) {
        OUT.deltaID[laneID] = uint8_t(strand_wg_offset);
    }
    OUT.baseID = baseID;

    // Task WG local
    uint sum_strandlet_count = subgroupBroadcast(strand_wg_offset + strandlet_count, 31);

    // Launch (Local Strandlet Count) number of Mesh Shader Workgroups
    EmitMeshTasksEXT(sum_strandlet_count, 1, 1);
}