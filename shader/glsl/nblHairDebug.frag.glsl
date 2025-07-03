#version 460

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#extension GL_GOOGLE_include_directive : enable
#include "inc/hairCommon.glsl"

layout (location = 0) in MeshDataDebug IN;

layout (location = 0) out vec4 out_color;

void main()
{
    out_color = IN.color;
}
