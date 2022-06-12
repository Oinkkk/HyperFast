//? #version 460
//? #extension GL_KHR_vulkan_glsl: enable
//? #define VERTEX_ATTRIB_POS true
//? #define VERTEX_ATTRIB_COLOR true

#include "Constant_Header.glsl"

#if (VERTEX_ATTRIB_POS == true)
layout(location = VERTEX_ATTRIB_LOCATION_POS) in vec3 inPos;
#endif

#if (VERTEX_ATTRIB_COLOR == true)
layout(location = VERTEX_ATTRIB_LOCATION_COLOR) in vec4 inColor;
#endif