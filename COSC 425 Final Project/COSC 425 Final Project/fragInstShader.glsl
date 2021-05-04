#version 430

in vec2 tc; // interpolated incoming texture coordinate
in vec3 varyingNormal;
in vec3 varyingLightDir1;
in vec3 varyingLightDir2;
in vec3 varyingVertPos;
in vec3 varyingHalfVector1;
in vec3 varyingHalfVector2;

in float instID;
out vec4 fragColor;

//in vec4 varyingColor;

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
uniform int modeltype;

layout (binding=0) uniform sampler2D samp;

void main(void)
{ // normalize the light, normal, and view vectors:
	vec3 L1 = normalize(varyingLightDir1);
	vec3 L2 = normalize(varyingLightDir2);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(-varyingVertPos);
	vec3 H1 = normalize(varyingHalfVector1);
	vec3 H2 = normalize(varyingHalfVector2);

	// compute light reflection vector with respect to N:
	//vec3 R = normalize(reflect(-L1, N));
	// get the angle between the light and surface normal:
	float cosTheta1 = dot(L1,N);
	float cosTheta2 = dot(L2,N);
	// angle between the view vector and reflected light:
	//float cosPhi = dot(V,R);
	float cosPhi1 = dot(H1,N);
	float cosPhi2 = dot(H2,N);

	// compute ADS contributions (per pixel), and combine to build output color:
	vec3 ambient1 = ((globalAmbient * material1.ambient) + (light1.ambient * material1.ambient)).xyz;
	vec3 diffuse1 = light1.diffuse.xyz * material1.diffuse.xyz * max(cosTheta1,0.0);
	vec3 specular1 = light1.specular.xyz * material1.specular.xyz * pow(max(cosPhi1,0.0), material1.shininess*3.0);

	vec3 ambient2 = ((globalAmbient * material2.ambient) + (light2.ambient * material2.ambient)).xyz;
	vec3 diffuse2 = light2.diffuse.xyz * material2.diffuse.xyz * max(cosTheta2,0.0);
	vec3 specular2 = light2.specular.xyz * material2.specular.xyz * pow(max(cosPhi2,0.0), material2.shininess*3.0);

	vec4 texColor = texture(samp, tc);
	float i = instID;
	float gFactor = i / 80.0f;
	vec4 sunColor = vec4(1.0f, gFactor+0.4f, 0.0f, 1.0f);

	vec4 light1 = vec4((ambient1 + diffuse1 + specular1), 1.0);
	vec4 light2 = vec4((ambient2 + diffuse2 + specular2), 1.0);
	
	if (modeltype == 1) {
		fragColor = sunColor * texColor;
	} else if (modeltype == 2) {
		fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f) * texColor;
	} else {
		fragColor = texColor * ((light1 / 2) + (light2 / 2));
	}
	
	
	if (fragColor.x > 1.0) { fragColor.x = 1.0; }
	if (fragColor.y > 1.0) { fragColor.y = 1.0; }
	if (fragColor.z > 1.0) { fragColor.z = 1.0; }
}