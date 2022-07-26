//? #version 460
//? #extension GL_KHR_vulkan_glsl: enable

const uint
	VERTEX_ATTRIB_LOCATION_POS		= 0U,
	VERTEX_ATTRIB_LOCATION_COLOR	= 1U,
	VERTEX_ATTRIB_LOCATION_NORMAL	= 2U
	;

const vec3
	VEC3_ZERO		= vec3(0.0f, 0.0f, 0.0f),
	VEC3_100		= vec3(1.0f, 0.0f, 0.0f);

const vec4
	COLOR_BLACK		= vec4(0.0f, 0.0f, 0.0f, 1.0f);