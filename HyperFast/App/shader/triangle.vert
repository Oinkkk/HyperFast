#version 460
//? #extension GL_KHR_vulkan_glsl: enable

const vec2 positions[] = vec2[]
(
	vec2(0.0f, -0.5f),
	vec2(-0.5f, 0.5f),
	vec2(0.5f, 0.5f)
);

const vec3 colors[3] = vec3[]
(
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f),
    vec3(0.0f, 0.0f, 1.0f)
);

layout(location = 0) out vec3 outColor;

void main()
{
	gl_Position = vec4(positions[gl_VertexIndex], 0.5f, 1.0f);
	outColor = colors[gl_VertexIndex];
}