#version 460
//? #extension GL_KHR_vulkan_glsl: enable

layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(inColor, 1.0f);
}