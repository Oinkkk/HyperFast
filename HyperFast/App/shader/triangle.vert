#version 460
//? #extension GL_KHR_vulkan_glsl: enable

#include "VertexAttrib_Header.vert"

//layout(binding = DESCRIPTOR_SET_BINDING_TRANSFORM) uniform UBTransform
//{
//	mat4 transform;
//}
//ubTransform;

layout(location = 0) out vec4 outColor;

void main()
{
	// gl_Position = (ubTransform.transform * vec4(inPos, 1.0f));
	gl_Position = vec4(inPos, 1.0f);
	outColor = inColor;
}