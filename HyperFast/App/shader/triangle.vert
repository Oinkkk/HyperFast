#version 460
//? #extension GL_KHR_vulkan_glsl: enable

#include "VertexAttrib_Header.vert"

layout(location = 0) out vec4 outColor;

void main()
{
	gl_Position = vec4(inPos, 1.0f);
	outColor = inColor;
}