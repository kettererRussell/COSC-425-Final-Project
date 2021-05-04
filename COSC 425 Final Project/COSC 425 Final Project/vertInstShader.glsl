#version 430

layout (location=0) in vec3 vertPos;
layout (location=1) in vec2 texCoord;
layout (location=2) in vec3 vertNormal;

out vec2 tc; // texture coordinate output to rasterizer for interpolation
//out vec4 varyingColor;
out float instID;
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
uniform mat4 m_matrix;
uniform mat4 v_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix; //for transforming normals
uniform float tf; //timefactor
uniform int modeltype; //1 for sun, 2 for particle explosion effect

layout (binding=0) uniform sampler2D samp; // not used in vertex shader

mat4 buildRotateX(float rad); // declaration of matrix transformation utility functions
mat4 buildRotateY(float rad); // GLSL requires functions to be declared prior to invocation
mat4 buildRotateZ(float rad);
mat4 buildTranslate(float x, float y, float z);
mat4 buildScale(float x, float y, float z);

mat4 buildSun(float givenID);
mat4 buildBurst(float givenID, float time);
mat4 buildExplosion(float givenID, float time);

void main(void)
{	instID = gl_InstanceID;
	float i = gl_InstanceID + tf;
	mat4 transf_matrix;

	if (modeltype == 1) {
		transf_matrix = buildSun(i);
	}
	else if (modeltype == 2) {
		transf_matrix = buildBurst(i, tf);
	}
	else {
		transf_matrix = buildExplosion(i, tf);
	}

	mat4 newM_matrix = m_matrix * transf_matrix;
	mat4 mv_matrix = v_matrix * newM_matrix;

	// output vertex position, light direction, and normal to the rasterizer for interpolation
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

mat4 buildScale(float x, float y, float z) {
	mat4 scale = mat4(x, 0.0, 0.0, 0.0,
					  0.0, y, 0.0, 0.0, 
					  0.0, 0.0, z, 0.0,
					  0.0, 0.0, 0.0, 1.0);
	return scale;
}

mat4 buildSun (float givenID) {
	float rotDir = 1;
	if (gl_InstanceID % 2 == 0) {
		rotDir = -1;
	}
	
	float a = sin(203.0 * givenID/8000.0) * 403.0;
	float b = cos(301.0 * givenID/4001.0) * 401.0;
	float c = sin(400.0 * givenID/6003.0) * 405.0;
	
	mat4 localRotX = buildRotateX(2.75*givenID*rotDir);
	mat4 localRotY = buildRotateY(0.75*givenID*rotDir);
	mat4 localRotZ = buildRotateZ(1.75*givenID*rotDir);
	mat4 trans = localRotX * localRotY * localRotZ;
	return trans;
}

mat4 buildBurst(float givenID, float time) {
	mat4 trans;
	float neg = 1;
	if (gl_InstanceID % 2 == 1) {
		neg = -1;
	}

	float angle_y = ((neg * (6.28319f/16.0f) * ((gl_InstanceID % 16) + 16.0f))/16);
	float angle_x = ((6.28319f/8.0f) * ((gl_InstanceID % 16) + 8.0f))/2;
	float distance_z = 25.0f*time;
	float scale_factor = -1.0f * time + 1.0f;
	if (scale_factor < 0.0f) {
		scale_factor = 0.0f;
	}

	mat4 localRotX = buildRotateX(angle_x);
	mat4 localRotY = buildRotateY(angle_y);
	mat4 localTranslation = buildTranslate(0.0f, 0.0f, distance_z);
	mat4 localScale = buildScale(scale_factor, scale_factor, scale_factor);

	trans = localRotX * localRotY;
	trans = trans * localTranslation;
	trans = localScale * trans;

	return trans;
}

mat4 buildExplosion(float givenID, float time) {
	float rotDir = (6.28319f/12.0f) * gl_InstanceID;
	
	float scalefactor = sin(time*2.0f)/1.2f;
	if (scalefactor < 0.0f) {
		scalefactor = 0.0f;
	}
	
	mat4 localRotX = buildRotateX(2.75*givenID);
	mat4 localRotY = buildRotateY(rotDir);
	//mat4 localRotZ = buildRotateZ(1.75*givenID*rotDir);
	mat4 localScale = buildScale(scalefactor, scalefactor, scalefactor);

	mat4 trans = localRotY * localRotX;
	trans = localScale * trans;
	
	return trans;
}