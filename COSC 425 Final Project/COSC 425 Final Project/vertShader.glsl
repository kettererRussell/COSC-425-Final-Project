#version 430

layout (location=0) in vec3 vertPos;
layout (location=1) in vec2 texCoord;
layout (location=2) in vec3 vertNormal;

out vec2 tc; // texture coordinate output to rasterizer for interpolation
//out vec4 varyingColor;

out vec3 varyingNormal; // eye-space vertex normal 
out vec3 varyingLightDir1; // vector pointing to the light 
out vec3 varyingLightDir2;
out vec3 varyingVertPos; // vertex position in eye space
out vec3 varyingHalfVector1;
out vec3 varyingHalfVector2;

struct PositionalLight
{	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec3 position;
};
struct Material
{	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light1;
uniform Material material1;
uniform PositionalLight light2;
uniform Material material2;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix; //for transforming normals
uniform int light_type;

layout (binding=0) uniform sampler2D samp; // not used in vertex shader

mat4 buildRotateX(float rad); // declaration of matrix transformation utility functions
mat4 buildRotateY(float rad); // GLSL requires functions to be declared prior to invocation
mat4 buildRotateZ(float rad);
mat4 buildTranslate(float x, float y, float z);

void main(void)
{	// output vertex position, light direction, and normal to the rasterizer for interpolation
	varyingVertPos = (mv_matrix * vec4(vertPos,1.0)).xyz;
	varyingLightDir1 = light1.position - varyingVertPos;
	varyingLightDir2 = light2.position - varyingVertPos;
	varyingNormal = (norm_matrix * vec4(vertNormal,1.0)).xyz;
	varyingHalfVector1 = (varyingLightDir1 + (-varyingVertPos)).xyz;
	varyingHalfVector2 = (varyingLightDir2 + (-varyingVertPos)).xyz;
	tc = texCoord;
	gl_Position = proj_matrix * mv_matrix * vec4(vertPos,1.0);
}

// builds and returns a matrix that performs a rotation around the X axis
mat4 buildRotateX(float rad)
{ mat4 xrot = mat4(1.0, 0.0, 0.0, 0.0,
					0.0, cos(rad), -sin(rad), 0.0,
					0.0, sin(rad), cos(rad), 0.0,
					0.0, 0.0, 0.0, 1.0 );
return xrot;
}
// builds and returns a matrix that performs a rotation around the Y axis
mat4 buildRotateY(float rad)
{ mat4 yrot = mat4(cos(rad), 0.0, sin(rad), 0.0,
					0.0, 1.0, 0.0, 0.0,
					-sin(rad), 0.0, cos(rad), 0.0,
					0.0, 0.0, 0.0, 1.0 );
	return yrot;
}
// builds and returns a matrix that performs a rotation around the Z axis
mat4 buildRotateZ(float rad)
{ mat4 zrot = mat4(cos(rad), -sin(rad), 0.0, 0.0,
					sin(rad), cos(rad), 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					0.0, 0.0, 0.0, 1.0 );
return zrot;
}
// utility function to build a translation matrix (from Chapter 3)
mat4 buildTranslate(float x, float y, float z)
{ mat4 trans = mat4(1.0, 0.0, 0.0, 0.0,
					0.0, 1.0, 0.0, 0.0,
					0.0, 0.0, 1.0, 0.0,
					x, y, z, 1.0 );
	return trans;
}