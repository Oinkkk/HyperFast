//? #version 460
//? #extension GL_KHR_vulkan_glsl: enable
//? #define VERTEX_ATTRIB_POS
//? #define VERTEX_ATTRIB_COLOR

#include "Constant_Header.glsl"

#ifdef VERTEX_ATTRIB_POS
layout(location = VERTEX_ATTRIB_LOCATION_POS) in vec3 inPos;
#else
const vec3 inPos = VEC3_ZERO;
#endif

#ifdef VERTEX_ATTRIB_COLOR
layout(location = VERTEX_ATTRIB_LOCATION_COLOR) in vec4 inColor;
#else
const vec4 inColor = COLOR_BLACK;
#endif