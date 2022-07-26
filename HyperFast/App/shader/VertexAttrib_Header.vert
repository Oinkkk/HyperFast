//? #version 460
//? #extension GL_KHR_vulkan_glsl: enable
//? #define VERTEX_ATTRIB_POS
//? #define VERTEX_ATTRIB_COLOR

#include "Constant_Header.glsl"

layout(location = VERTEX_ATTRIB_LOCATION_POS) in vec3 inPos;
layout(location = VERTEX_ATTRIB_LOCATION_COLOR) in vec4 inColor;
layout(location = VERTEX_ATTRIB_LOCATION_NORMAL) in vec3 inNormal;