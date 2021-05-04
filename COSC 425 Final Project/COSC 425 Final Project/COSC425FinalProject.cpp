#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <glm\glm\glm.hpp>
#include <glm\glm\gtc\type_ptr.hpp>
#include <glm\glm\gtc\matrix_transform.hpp>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stack>
#include <vector>
#include <math.h>
#include <SOIL2/soil2.h>

#include "Laser.h"
#include "BurstParticle.h"
#include "Planet.h"
#include "Demon.h"
#include "SpaceShip.h"
#include "C:\\Users\\russk\\Documents\\Visual Studio 2017\Projects\COSC 425\COSC 425 Final Project\COSC 425 Final Project\SpaceShip.h"
#include "C:\\Users\\russk\\Documents\\Visual Studio 2017\\Projects\\COSC 425\\COSC 425 Book Program 7_2\\COSC 425 Book Program 7_2\\ImportedModel.h"

using namespace std;

//all planet textures, which does not include the sun, are taken from here: http://planetpixelemporium.com/pluto.html
//bump maps were done on some planets, but not all, and they were done by me, in blender.


#define numVAOs 1
#define numVBOs 29

double lastTime, diffTime, thrustTime;

float cameraX = 0.0f; float cameraY = 0.0f; float cameraZ = 6.0f;
float cameraXspeed = 0.0f; float cameraYspeed = 0.0f; float cameraZspeed = 0.0f;
float cameraDist = 12.0f;
float cameraRotX = 0.0f; float cameraRotY = 0.0f; float cameraRotZ = 0.0f;
float cameraRotXspeed = 0.0f; float cameraRotYspeed = 0.0f; float cameraRotZspeed = 0.0f;
float cameraSpeedMultiplier = 1.0f;
float cameraRotYadd = 0.0f;
float cameraXdiff = 0.0f; float cameraZdiff = 0.0f;

float poolTableLocX, poolTableLocY, poolTableLocZ;

 // in top-level declarations
ImportedModel cubeModel("Assets\\Models\\cube.obj");
ImportedModel planeModel("Assets\\Models\\plane2D.obj");
ImportedModel coneModel("Assets\\Models\\thruster_burst.obj");
ImportedModel cylinderModel("Assets\\Models\\cylinder.obj");
ImportedModel mpMarsModel("Assets\\Models\\marsMedPoly.obj");
ImportedModel spaceShipModel("Assets\\Models\\SpaceShip_v1_tris.obj");
ImportedModel UFOModel("Assets\\Models\\UFO_tris.obj");
ImportedModel LaserModel("Assets\\Models\\laser_tris.obj");

SpaceShip player;
glm::vec4 playerCoords;
bool playerForwardThrust, playerBackwardThrust, playerCWthrust, playerCCWthrust;

vector<Laser> lasers;
glm::vec4 laserCoords;

vector<BurstParticle> bursts;
glm::vec4 burstCoords;

vector<Planet> planets;
glm::vec4 planetCoords;

vector<Demon> demons;
glm::vec4 demonCoords;

GLuint renderingProgram;
GLuint instancingProgram;
GLuint vao[numVAOs];
GLuint vbo[numVBOs];

void printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char *log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char *)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}
void printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char *log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char *)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}
bool checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}
string readShaderSource(const char *filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}
GLuint createShaderProgram(const char *vS, const char *fS) {
	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	string vertShaderStr = readShaderSource(vS);
	string fragShaderStr = readShaderSource(fS);
	const char *vertShaderSrc = vertShaderStr.c_str();
	const char *fragShaderSrc = fragShaderStr.c_str();
	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1) {
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}

	glCompileShader(fShader);
	checkOpenGLError();
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	GLuint vfProgram = glCreateProgram();

	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	glLinkProgram(vfProgram);
	glLinkProgram(vfProgram);
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1) {
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	return vfProgram;
}
GLuint loadTexture(const char *texImagePath) {
	GLuint textureID;
	textureID = SOIL_load_OGL_texture(texImagePath,
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
		GLfloat anisoSetting = 0.0f;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisoSetting);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisoSetting);
	}
	if (textureID == 0) cout << "could not find texture file" << texImagePath << endl;
	return textureID;
}

bool check;
bool windowOpen = true;
void printMatrix(glm::mat4 matrix) {
	if (check == true) {
		cout << matrix[0][0] << " " << matrix[0][1] << " " << matrix[0][2] << " " << matrix[0][3] << " " << endl;
		cout << matrix[1][0] << " " << matrix[1][1] << " " << matrix[1][2] << " " << matrix[1][3] << " " << endl;
		cout << matrix[2][0] << " " << matrix[2][1] << " " << matrix[2][2] << " " << matrix[2][3] << " " << endl;
		cout << matrix[3][0] << " " << matrix[3][1] << " " << matrix[3][2] << " " << matrix[3][3] << " " << endl;
		check = false;

		glm::vec4 rayPoint = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		glm::mat4 rotMat = glm::rotate(glm::mat4(1.0f), glm::radians(45.1f), glm::vec3(0.0f, 1.0f, 0.0f));
		rotMat *= glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 tranMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		rayPoint = (rotMat * tranMat) * rayPoint;
		//rayPoint = (tranMat * rotMat) * rayPoint;

		cout << "X: " << rayPoint.x << " Y: " << rayPoint.y << " Z: " << rayPoint.z << endl;
	}
}

float* skyboxAmbient() { static float a[4] = { 0.5f, 0.1f, -0.1f, 1 }; return (float*)a; }
float* zeroMatLight() { static float a[4] = { 0.0f, 0.0f, 0.0f, 1 }; return (float*)a; }
// GOLD material - ambient, diffuse, specular, and shininess
float* goldAmbient() { static float a[4] = { 0.2473f, 0.1995f, 0.0745f, 1 }; return (float*)a; }
float* goldDiffuse() { static float a[4] = { 0.7516f, 0.6065f, 0.2265f, 1 }; return (float*)a; }
float* goldSpecular() { static float a[4] = { 0.6283f, 0.5559f, 0.3661f, 1 }; return (float*)a; }
float goldShininess() { return 51.2f; }
// SILVER material - ambient, diffuse, specular, and shininess
float* silverAmbient() { static float a[4] = { 0.1923f, 0.1923f, 0.1923f, 1 }; return (float*)a; }
float* silverDiffuse() { static float a[4] = { 0.5075f, 0.5075f, 0.5075f, 1 }; return (float*)a; }
float* silverSpecular() { static float a[4] = { 0.5083f, 0.5083f, 0.5083f, 1 }; return (float*)a; }
float silverShininess() { return 51.2f; }
// BRONZE material - ambient, diffuse, specular, and shininess
float* bronzeAmbient() { static float a[4] = { 0.2125f, 0.1275f, 0.0540f, 1 }; return (float*)a; }
float* bronzeDiffuse() { static float a[4] = { 0.7140f, 0.4284f, 0.1814f, 1 }; return (float*)a; }
float* bronzeSpecular() { static float a[4] = { 0.3936f, 0.2719f, 0.1667f, 1 }; return (float*)a; }
float bronzeShininess() { return 25.6f; }
// RUBBER material
float* rubberAmbient() { static float a[4] = { 0.02f, 0.02f, 0.02, 1 }; return (float*)a; }
float* rubberDiffuse() { static float a[4] = { 0.01f, 0.01, 0.01f, 1 }; return (float*)a; }
float* rubberSpecular() { static float a[4] = { 0.4f, 0.4f, 0.4f, 1 }; return (float*)a; }
float rubberShininess() { return 10.0f; }

GLuint mLoc, vLoc, mvLoc, tfLoc, mtLoc, projLoc, nLoc;
GLuint earthMapTexture,
marsMapTexture, mercuryTexture, venusTexture, jupiterTexture, 
saturnTexture, uranusTexture, neptuneTexture, plutoTexture,
feltTexture, spaceShipTexture, spaceShipThrusterTexture, 
laserBaseTexture, UFOTexture, lavaTexture, skyboxTexture;
GLuint globalAmbLoc, amb1Loc, diff1Loc, spec1Loc, pos1Loc, mAmb1Loc, mDiff1Loc, mSpec1Loc, mShi1Loc,
amb2Loc, diff2Loc, spec2Loc, pos2Loc, mAmb2Loc, mDiff2Loc, mSpec2Loc, mShi2Loc, ltLoc;
int width, height;
float aspect;
float timeFactor;

glm::mat4 pMat, tMat, rMat, vMat, mMat, mvMat, invTrMat;
glm::vec3 currentLightPos1, currentLightPos2, lightPos1V, lightPos2V; // light position as Vector3f, in both model and view space
float lightPos1[3]; // light position as float array
float lightPos2[3];
// initial light location
glm::vec3 initialLightLoc1 = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 initialLightLoc2 = glm::vec3(0.0f, 0.0f, -19.8f);
// white light properties
float globalAmbient[4] = { 1.8f, 1.0f, 1.0f, 1.0f };

float lightAmbient1[4] = { 0.1f, 0.0f, 0.0f, 1.0f };
float lightDiffuse1[4] = { 3.0f, 2.0f, 2.0f, 1.0f };
float lightSpecular1[4] = { 2.0f, 1.0f, 1.0f, 1.0f };

float lightAmbient2[4] = { 0.1f, 0.0f, 0.0f, 1.0f };
float lightDiffuse2[4] = { 3.0f, 2.0f, 2.0f, 1.0f };
float lightSpecular2[4] = { 2.0f, 1.0f, 1.0f, 1.0f };
// gold material properties
float* matAmb1 = silverAmbient();
float* matDif1 = silverDiffuse();
float* matSpe1 = silverSpecular();
float matShi1 = silverShininess();

float* matAmb2 = silverAmbient();
float* matDif2 = silverDiffuse();
float* matSpe2 = silverSpecular();
float matShi2 = silverShininess();

float cubDim = 1.0f;
void setupVertices(void) {
	float cubePositions[108] = {
		-cubDim, cubDim, -cubDim, -cubDim, -cubDim, -cubDim, cubDim, -cubDim, -cubDim, //back side, top left, bottom left, bottom right
		cubDim, -cubDim, -cubDim, cubDim, cubDim, -cubDim, -cubDim, cubDim, -cubDim, //" ", bottom right, top right, top left
		cubDim, -cubDim, -cubDim, cubDim, -cubDim, cubDim, cubDim, cubDim, -cubDim, //right side, 
		cubDim, -cubDim, cubDim, cubDim, cubDim, cubDim, cubDim, cubDim, -cubDim,
		cubDim, -cubDim, cubDim, -cubDim, -cubDim, cubDim, cubDim, cubDim, cubDim, //front side, 
		-cubDim, -cubDim, cubDim, -cubDim, cubDim, cubDim, cubDim, cubDim, cubDim,
		-cubDim, -cubDim, cubDim, -cubDim, -cubDim, -cubDim, -cubDim, cubDim, cubDim, //left side,
		-cubDim, -cubDim, -cubDim, -cubDim, cubDim, -cubDim, -cubDim, cubDim, cubDim,
		-cubDim, -cubDim, cubDim, cubDim, -cubDim, cubDim, cubDim, -cubDim, -cubDim, //bottom side,
		cubDim, -cubDim, -cubDim, -cubDim, -cubDim, -cubDim, -cubDim, -cubDim, cubDim,
		-cubDim, cubDim, -cubDim, cubDim, cubDim, -cubDim, cubDim, cubDim, cubDim, //top side, 
		cubDim, cubDim, cubDim, -cubDim, cubDim, cubDim, -cubDim, cubDim, -cubDim
	};

	float skyboxCubeTexCoords[72] = {
		1.00f, 0.65f, 1.00f, 0.34f, 0.75f, 0.34f, // back face lower right
		0.75f, 0.33f, 0.75f, 0.66f, 1.00f, 0.66f, // back face upper left
		0.75f, 0.34f, 0.50f, 0.34f, 0.75f, 0.65f, // right face lower right
		0.50f, 0.33f, 0.50f, 0.66f, 0.75f, 0.66f, // right face upper left
		0.50f, 0.34f, 0.25f, 0.34f, 0.50f, 0.65f, // front face lower right
		0.25f, 0.33f, 0.25f, 0.66f, 0.50f, 0.66f, // front face upper left
		0.25f, 0.34f, 0.00f, 0.34f, 0.25f, 0.65f, // left face lower right
		0.00f, 0.33f, 0.00f, 0.66f, 0.25f, 0.66f, // left face upper left
		0.26f, 0.33f, 0.49f, 0.33f, 0.49f, 0.00f, // bottom face upper right
		0.49f, 0.00f, 0.26f, 0.00f, 0.26f, 0.33f, // bottom face lower left
		0.26f, 1.00f, 0.49f, 1.00f, 0.49f, 0.66f, // top face upper right
		0.49f, 0.66f, 0.26f, 0.66f, 0.26f, 1.00f // top face lower left
	};

	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	glGenBuffers(numVBOs, vbo);

	std::vector<glm::vec3> cubeVert = cubeModel.getVertices();
	std::vector<glm::vec2> cubeTex = cubeModel.getTextureCoords();
	std::vector<glm::vec3> cubeNorm = cubeModel.getNormals();
	int cubeNumObjVertices = cubeModel.getNumVertices();
	std::vector<float> cubeModel_pvalues; // vertex positions
	std::vector<float> cubeModel_tvalues; // texture coordinates
	std::vector<float> cubeModel_nvalues; // normal vectors
	for (int i = 0; i < cubeNumObjVertices; i++) {
		cubeModel_pvalues.push_back((cubeVert[i]).x);
		cubeModel_pvalues.push_back((cubeVert[i]).y);
		cubeModel_pvalues.push_back((cubeVert[i]).z);
		cubeModel_tvalues.push_back((cubeTex[i]).s);
		cubeModel_tvalues.push_back((cubeTex[i]).t);
		cubeModel_nvalues.push_back((cubeNorm[i]).x);
		cubeModel_nvalues.push_back((cubeNorm[i]).y);
		cubeModel_nvalues.push_back((cubeNorm[i]).z);
	}
	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, cubeModel_pvalues.size() * 4, &cubeModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, cubeModel_tvalues.size() * 4, &cubeModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, cubeModel_nvalues.size() * 4, &cubeModel_nvalues[0], GL_STATIC_DRAW);

	std::vector<glm::vec3> planeVert = planeModel.getVertices();
	std::vector<glm::vec2> planeTex = planeModel.getTextureCoords();
	std::vector<glm::vec3> planeNorm = planeModel.getNormals();
	int planeNumObjVertices = planeModel.getNumVertices();
	std::vector<float> planeModel_pvalues; // vertex positions
	std::vector<float> planeModel_tvalues; // texture coordinates
	std::vector<float> planeModel_nvalues; // normal vectors
	for (int i = 0; i < planeNumObjVertices; i++) {
		planeModel_pvalues.push_back((planeVert[i]).x);
		planeModel_pvalues.push_back((planeVert[i]).y);
		planeModel_pvalues.push_back((planeVert[i]).z);
		planeModel_tvalues.push_back((planeTex[i]).s);
		planeModel_tvalues.push_back((planeTex[i]).t);
		planeModel_nvalues.push_back((planeNorm[i]).x);
		planeModel_nvalues.push_back((planeNorm[i]).y);
		planeModel_nvalues.push_back((planeNorm[i]).z);
	}
	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, planeModel_pvalues.size() * 4, &planeModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, planeModel_tvalues.size() * 4, &planeModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, planeModel_nvalues.size() * 4, &planeModel_nvalues[0], GL_STATIC_DRAW);

	std::vector<glm::vec3> coneVert = coneModel.getVertices();
	std::vector<glm::vec2> coneTex = coneModel.getTextureCoords();
	std::vector<glm::vec3> coneNorm = coneModel.getNormals();
	int coneNumObjVertices = coneModel.getNumVertices();
	std::vector<float> coneModel_pvalues; // vertex positions
	std::vector<float> coneModel_tvalues; // texture coordinates
	std::vector<float> coneModel_nvalues; // normal vectors
	for (int i = 0; i < coneNumObjVertices; i++) {
		coneModel_pvalues.push_back((coneVert[i]).x);
		coneModel_pvalues.push_back((coneVert[i]).y);
		coneModel_pvalues.push_back((coneVert[i]).z);
		coneModel_tvalues.push_back((coneTex[i]).s);
		coneModel_tvalues.push_back((coneTex[i]).t);
		coneModel_nvalues.push_back((coneNorm[i]).x);
		coneModel_nvalues.push_back((coneNorm[i]).y);
		coneModel_nvalues.push_back((coneNorm[i]).z);
	}
	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, coneModel_pvalues.size() * 4, &coneModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
	glBufferData(GL_ARRAY_BUFFER, coneModel_tvalues.size() * 4, &coneModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
	glBufferData(GL_ARRAY_BUFFER, coneModel_nvalues.size() * 4, &coneModel_nvalues[0], GL_STATIC_DRAW);

	std::vector<glm::vec3> cylinderVert = cylinderModel.getVertices();
	std::vector<glm::vec2> cylinderTex = cylinderModel.getTextureCoords();
	std::vector<glm::vec3> cylinderNorm = cylinderModel.getNormals();
	int cylinderNumObjVertices = cylinderModel.getNumVertices();
	std::vector<float> cylinderModel_pvalues; // vertex positions
	std::vector<float> cylinderModel_tvalues; // texture coordinates
	std::vector<float> cylinderModel_nvalues; // normal vectors
	for (int i = 0; i < cylinderNumObjVertices; i++) {
		cylinderModel_pvalues.push_back((cylinderVert[i]).x);
		cylinderModel_pvalues.push_back((cylinderVert[i]).y);
		cylinderModel_pvalues.push_back((cylinderVert[i]).z);
		cylinderModel_tvalues.push_back((cylinderTex[i]).s);
		cylinderModel_tvalues.push_back((cylinderTex[i]).t);
		cylinderModel_nvalues.push_back((cylinderNorm[i]).x);
		cylinderModel_nvalues.push_back((cylinderNorm[i]).y);
		cylinderModel_nvalues.push_back((cylinderNorm[i]).z);
	}
	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[12]);
	glBufferData(GL_ARRAY_BUFFER, cylinderModel_pvalues.size() * 4, &cylinderModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[13]);
	glBufferData(GL_ARRAY_BUFFER, cylinderModel_tvalues.size() * 4, &cylinderModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[14]);
	glBufferData(GL_ARRAY_BUFFER, cylinderModel_nvalues.size() * 4, &cylinderModel_nvalues[0], GL_STATIC_DRAW);

	std::vector<glm::vec3> mpMarsVert = mpMarsModel.getVertices();
	std::vector<glm::vec2> mpMarsTex = mpMarsModel.getTextureCoords();
	std::vector<glm::vec3> mpMarsNorm = mpMarsModel.getNormals();
	int mpMarsModelNumObjVertices = mpMarsModel.getNumVertices();
	std::vector<float> mpMarsModel_pvalues; // vertex positions
	std::vector<float> mpMarsModel_tvalues; // texture coordinates
	std::vector<float> mpMarsModel_nvalues; // normal vectors
	for (int i = 0; i < mpMarsModelNumObjVertices; i++) {
		mpMarsModel_pvalues.push_back((mpMarsVert[i]).x);
		mpMarsModel_pvalues.push_back((mpMarsVert[i]).y);
		mpMarsModel_pvalues.push_back((mpMarsVert[i]).z);
		mpMarsModel_tvalues.push_back((mpMarsTex[i]).s);
		mpMarsModel_tvalues.push_back((mpMarsTex[i]).t);
		mpMarsModel_nvalues.push_back((mpMarsNorm[i]).x);
		mpMarsModel_nvalues.push_back((mpMarsNorm[i]).y);
		mpMarsModel_nvalues.push_back((mpMarsNorm[i]).z);
	}
	
	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[15]);
	glBufferData(GL_ARRAY_BUFFER, mpMarsModel_pvalues.size() * 4, &mpMarsModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[16]);
	glBufferData(GL_ARRAY_BUFFER, mpMarsModel_tvalues.size() * 4, &mpMarsModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[17]);
	glBufferData(GL_ARRAY_BUFFER, mpMarsModel_nvalues.size() * 4, &mpMarsModel_nvalues[0], GL_STATIC_DRAW);

	std::vector<glm::vec3> spaceShipVert = spaceShipModel.getVertices();
	std::vector<glm::vec2> spaceShipTex = spaceShipModel.getTextureCoords();
	std::vector<glm::vec3> spaceShipNorm = spaceShipModel.getNormals();
	int spaceShipModelNumObjVertices = spaceShipModel.getNumVertices();
	std::vector<float> spaceShipModel_pvalues; // vertex positions
	std::vector<float> spaceShipModel_tvalues; // texture coordinates
	std::vector<float> spaceShipModel_nvalues; // normal vectors
	for (int i = 0; i < spaceShipModelNumObjVertices; i++) {
		spaceShipModel_pvalues.push_back((spaceShipVert[i]).x);
		spaceShipModel_pvalues.push_back((spaceShipVert[i]).y);
		spaceShipModel_pvalues.push_back((spaceShipVert[i]).z);
		spaceShipModel_tvalues.push_back((spaceShipTex[i]).s);
		spaceShipModel_tvalues.push_back((spaceShipTex[i]).t);
		spaceShipModel_nvalues.push_back((spaceShipNorm[i]).x);
		spaceShipModel_nvalues.push_back((spaceShipNorm[i]).y);
		spaceShipModel_nvalues.push_back((spaceShipNorm[i]).z);
	}

	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[18]);
	glBufferData(GL_ARRAY_BUFFER, spaceShipModel_pvalues.size() * 4, &spaceShipModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[19]);
	glBufferData(GL_ARRAY_BUFFER, spaceShipModel_tvalues.size() * 4, &spaceShipModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[20]);
	glBufferData(GL_ARRAY_BUFFER, spaceShipModel_nvalues.size() * 4, &spaceShipModel_nvalues[0], GL_STATIC_DRAW);

	//skybox
	glBindBuffer(GL_ARRAY_BUFFER, vbo[21]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubePositions), cubePositions, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[22]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxCubeTexCoords), skyboxCubeTexCoords, GL_STATIC_DRAW);

	std::vector<glm::vec3> UFOVert = UFOModel.getVertices();
	std::vector<glm::vec2> UFOTex = UFOModel.getTextureCoords();
	std::vector<glm::vec3> UFONorm = UFOModel.getNormals();
	int UFOModelNumObjVertices = UFOModel.getNumVertices();
	std::vector<float> UFOModel_pvalues; // vertex positions
	std::vector<float> UFOModel_tvalues; // texture coordinates
	std::vector<float> UFOModel_nvalues; // normal vectors
	for (int i = 0; i < UFOModelNumObjVertices; i++) {
		UFOModel_pvalues.push_back((UFOVert[i]).x);
		UFOModel_pvalues.push_back((UFOVert[i]).y);
		UFOModel_pvalues.push_back((UFOVert[i]).z);
		UFOModel_tvalues.push_back((UFOTex[i]).s);
		UFOModel_tvalues.push_back((UFOTex[i]).t);
		UFOModel_nvalues.push_back((UFONorm[i]).x);
		UFOModel_nvalues.push_back((UFONorm[i]).y);
		UFOModel_nvalues.push_back((UFONorm[i]).z);
	}

	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[23]);
	glBufferData(GL_ARRAY_BUFFER, UFOModel_pvalues.size() * 4, &UFOModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[24]);
	glBufferData(GL_ARRAY_BUFFER, UFOModel_tvalues.size() * 4, &UFOModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[25]);
	glBufferData(GL_ARRAY_BUFFER, UFOModel_nvalues.size() * 4, &UFOModel_nvalues[0], GL_STATIC_DRAW);

	std::vector<glm::vec3> LaserVert = LaserModel.getVertices();
	std::vector<glm::vec2> LaserTex = LaserModel.getTextureCoords();
	std::vector<glm::vec3> LaserNorm = LaserModel.getNormals();
	int LaserModelNumObjVertices = LaserModel.getNumVertices();
	std::vector<float> LaserModel_pvalues; // vertex positions
	std::vector<float> LaserModel_tvalues; // texture coordinates
	std::vector<float> LaserModel_nvalues; // normal vectors
	for (int i = 0; i < LaserModelNumObjVertices; i++) {
		LaserModel_pvalues.push_back((LaserVert[i]).x);
		LaserModel_pvalues.push_back((LaserVert[i]).y);
		LaserModel_pvalues.push_back((LaserVert[i]).z);
		LaserModel_tvalues.push_back((LaserTex[i]).s);
		LaserModel_tvalues.push_back((LaserTex[i]).t);
		LaserModel_nvalues.push_back((LaserNorm[i]).x);
		LaserModel_nvalues.push_back((LaserNorm[i]).y);
		LaserModel_nvalues.push_back((LaserNorm[i]).z);
	}

	// VBO for vertex locations
	glBindBuffer(GL_ARRAY_BUFFER, vbo[26]);
	glBufferData(GL_ARRAY_BUFFER, LaserModel_pvalues.size() * 4, &LaserModel_pvalues[0], GL_STATIC_DRAW);
	// VBO for texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[27]);
	glBufferData(GL_ARRAY_BUFFER, LaserModel_tvalues.size() * 4, &LaserModel_tvalues[0], GL_STATIC_DRAW);
	// VBO for normal vectors
	glBindBuffer(GL_ARRAY_BUFFER, vbo[28]);
	glBufferData(GL_ARRAY_BUFFER, LaserModel_nvalues.size() * 4, &LaserModel_nvalues[0], GL_STATIC_DRAW);
}

void collisionDetection(double cT) {
	float laserX, laserZ, laserSize; //laser coords and laser size
	for (int i = 0; i < demons.size(); i++) { //restructure to look for demons first then lasers second
		//cout << "hello" << endl;
		//cout << playerCoords.x << " " << playerCoords.z << endl;
		 //needs another for statement of demons to check
		for (int j = 0; j < lasers.size(); j++) {
			laserX = lasers[j].getCoords().x;
			laserZ = lasers[j].getCoords().z;
			laserSize = lasers[j].getSize();
			if (glm::distance(demons[i].getCoords(), lasers[j].getCoords()) < laserSize*2.0f) {
				/*cout << "Demon X: " << demonCoords.x << " Laser X: " << laserX;
				cout << " Demon Z: " << demonCoords.z << " Laser Z: " << laserZ;
				cout << " Laser Hit Demon " << endl;*/
				lasers[j].killLaser();
				BurstParticle nBurst(glm::vec4(laserX, 0.0f, laserZ, 1.0f), cT, 32, 2);
				bursts.push_back(nBurst);
				demons[i].takeDamage(lasers[j].getDamage());
				if (demons[i].getAliveStatus() == false) {
					nBurst.setNumInst(12);
					nBurst.setID(3);
					nBurst.setCoords(demons[i].getCoords());
					bursts.push_back(nBurst);
				}
			}
		}
		if (glm::distance(demons[i].getCoords(), playerCoords) < 2.0f) {
			player.takeDamage(demons[i].getDamage());
		}
	}


}

void setupPlanets() {
	int numPlanets = 9;
	glm::vec4 pCoords;
	float pDist;

	for (int i = 0; i < numPlanets; i++) {
		pDist = (float)(i*20)+25.0f;
		float rotYfactor;
		float planetSize = 8.0f;
		if (i % 2 == 0) {
			rotYfactor = (1.0f+(float)(i^2))/((float)i);
		}
		else {
			rotYfactor = -(1.0f + (float)(i ^ 2)) / ((float)i);
		}
		if (i == 0) { rotYfactor = 1.0f; }
		rotYfactor = rotYfactor *0.3f;
		pCoords = glm::vec4(pDist, 0.0f, 0.0f, 1.0f);
		Planet newPlanet(pCoords, 0.0f, 8.0f, pDist, rotYfactor);
		if (i % 9 == 0) {
			newPlanet.setTexture(mercuryTexture);
			planetSize = 2.0f;
		}
		else if (i % 9 == 1) {
			newPlanet.setTexture(venusTexture);
			planetSize = 5.5f;
		}
		else if (i % 9 == 2) {
			newPlanet.setTexture(earthMapTexture);
			planetSize = 6.0f;
		}
		else if (i % 9 == 3) {
			newPlanet.setTexture(marsMapTexture);
			planetSize = 3.0f;
		}
		else if (i % 9 == 4) {
			newPlanet.setTexture(jupiterTexture);
			planetSize = 22.0f;
		}
		else if (i % 9 == 5) {
			newPlanet.setTexture(saturnTexture);
			planetSize = 18.0f;
		}
		else if (i % 9 == 6) {
			newPlanet.setTexture(uranusTexture);
			planetSize = 10.0f;
		}
		else if (i % 9 == 7) {
			newPlanet.setTexture(neptuneTexture);
			planetSize = 9.0f;
		}
		else if (i % 9 == 8) {
			newPlanet.setTexture(plutoTexture);
			planetSize = 1.0f;
		}
		else {
			newPlanet.setTexture(marsMapTexture);
		}
		newPlanet.setSize(planetSize);
		planets.push_back(newPlanet);

	}
}

void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram("vertShader.glsl", "fragShader.glsl");
	instancingProgram = createShaderProgram("vertInstShader.glsl", "fragInstShader.glsl");

	srand(time(NULL));
	thrustTime = 0.0;

	playerCoords = player.getCoords();
	playerCoords.z = -150.0f;
	player.setCoords(playerCoords);

	diffTime = 0.0;
	lastTime = glfwGetTime();

	cameraX = 0.0f; cameraY = 12.0f; cameraZ = 16.0f;
	cameraRotX = 20.0f; cameraRotY = 0.0f; cameraRotZ = 0.0f;

	check = true;
	setupVertices();
	
	feltTexture = loadTexture("Assets\\Textures\\blueFelt.jpg");
	lavaTexture = loadTexture("Assets\\Textures\\lava.jpg");

	mercuryTexture = loadTexture("Assets\\Textures\\mercury1k.jpg");
	venusTexture = loadTexture("Assets\\Textures\\venus1k.jpg");
	earthMapTexture = loadTexture("Assets\\Textures\\earth1k.jpg");
	marsMapTexture = loadTexture("Assets\\Textures\\mars2k.jpg");
	jupiterTexture = loadTexture("Assets\\Textures\\jupiter1k.jpg");
	saturnTexture = loadTexture("Assets\\Textures\\saturn1k.jpg");
	uranusTexture = loadTexture("Assets\\Textures\\uranus1k.jpg");
	neptuneTexture = loadTexture("Assets\\Textures\\neptune1k.jpg");
	plutoTexture = loadTexture("Assets\\Textures\\pluto1k.jpg");

	spaceShipTexture = loadTexture("Assets\\Textures\\spaceShip.png");
	spaceShipThrusterTexture = loadTexture("Assets\\Textures\\spaceShip_thrusters.png");
	
	laserBaseTexture = loadTexture("Assets\\Textures\\Laser_base.png");

	UFOTexture = loadTexture("Assets\\Textures\\UFOsteel.jpg");

	skyboxTexture = loadTexture("Assets\\Textures\\spaceBox1.png");

	setupPlanets();
}

void installSkyboxLights(glm::mat4 vMatrix) {

	matAmb1 = skyboxAmbient();
	matDif1 = zeroMatLight();
	matSpe1 = zeroMatLight();
	matShi1 = 200.0f;

	matAmb2 = skyboxAmbient();
	matDif2 = zeroMatLight();
	matSpe2 = zeroMatLight();
	matShi2 = 200.0f;

	lightPos1V = glm::vec3(vMatrix * glm::vec4(currentLightPos1, 1.0));
	lightPos2V = glm::vec3(vMatrix * glm::vec4(currentLightPos1, 1.0));
	lightPos1[0] = lightPos1V.x;
	lightPos1[1] = lightPos1V.y;
	lightPos1[2] = lightPos1V.z;
	lightPos2[0] = lightPos2V.x;
	lightPos2[1] = lightPos2V.y;
	lightPos2[2] = lightPos2V.z;

	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, amb1Loc, 1, lightAmbient1);
	glProgramUniform4fv(renderingProgram, diff1Loc, 1, lightDiffuse1);
	glProgramUniform4fv(renderingProgram, spec1Loc, 1, lightSpecular1);
	glProgramUniform3fv(renderingProgram, pos1Loc, 1, lightPos1);
	glProgramUniform4fv(renderingProgram, mAmb1Loc, 1, matAmb1);
	glProgramUniform4fv(renderingProgram, mDiff1Loc, 1, matDif1);
	glProgramUniform4fv(renderingProgram, mSpec1Loc, 1, matSpe1);
	glProgramUniform1f(renderingProgram, mShi1Loc, matShi1);

	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, amb2Loc, 1, lightAmbient2);
	glProgramUniform4fv(renderingProgram, diff2Loc, 1, lightDiffuse2);
	glProgramUniform4fv(renderingProgram, spec2Loc, 1, lightSpecular2);
	glProgramUniform3fv(renderingProgram, pos2Loc, 1, lightPos2);
	glProgramUniform4fv(renderingProgram, mAmb2Loc, 1, matAmb2);
	glProgramUniform4fv(renderingProgram, mDiff2Loc, 1, matDif2);
	glProgramUniform4fv(renderingProgram, mSpec2Loc, 1, matSpe2);
	glProgramUniform1f(renderingProgram, mShi2Loc, matShi2);
}

void setLightLoc() {
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	amb1Loc = glGetUniformLocation(renderingProgram, "light1.ambient");
	diff1Loc = glGetUniformLocation(renderingProgram, "light1.diffuse");
	spec1Loc = glGetUniformLocation(renderingProgram, "light1.specular");
	pos1Loc = glGetUniformLocation(renderingProgram, "light1.position");
	mAmb1Loc = glGetUniformLocation(renderingProgram, "material1.ambient");
	mDiff1Loc = glGetUniformLocation(renderingProgram, "material1.diffuse");
	mSpec1Loc = glGetUniformLocation(renderingProgram, "material1.specular");
	mShi1Loc = glGetUniformLocation(renderingProgram, "material1.shininess");

	amb2Loc = glGetUniformLocation(renderingProgram, "light2.ambient");
	diff2Loc = glGetUniformLocation(renderingProgram, "light2.diffuse");
	spec2Loc = glGetUniformLocation(renderingProgram, "light2.specular");
	pos2Loc = glGetUniformLocation(renderingProgram, "light2.position");
	mAmb2Loc = glGetUniformLocation(renderingProgram, "material2.ambient");
	mDiff2Loc = glGetUniformLocation(renderingProgram, "material2.diffuse");
	mSpec2Loc = glGetUniformLocation(renderingProgram, "material2.specular");
	mShi2Loc = glGetUniformLocation(renderingProgram, "material2.shininess");
}

void installLightsNoLoc(glm::mat4 vMatrix) {
	lightPos1V = glm::vec3(vMatrix * glm::vec4(currentLightPos1, 1.0));
	lightPos2V = glm::vec3(vMatrix * glm::vec4(currentLightPos2, 1.0));
	lightPos1[0] = lightPos1V.x;
	lightPos1[1] = lightPos1V.y;
	lightPos1[2] = lightPos1V.z;
	lightPos2[0] = lightPos2V.x;
	lightPos2[1] = lightPos2V.y;
	lightPos2[2] = lightPos2V.z;

	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, amb1Loc, 1, lightAmbient1);
	glProgramUniform4fv(renderingProgram, diff1Loc, 1, lightDiffuse1);
	glProgramUniform4fv(renderingProgram, spec1Loc, 1, lightSpecular1);
	glProgramUniform3fv(renderingProgram, pos1Loc, 1, lightPos1);
	glProgramUniform4fv(renderingProgram, mAmb1Loc, 1, matAmb1);
	glProgramUniform4fv(renderingProgram, mDiff1Loc, 1, matDif1);
	glProgramUniform4fv(renderingProgram, mSpec1Loc, 1, matSpe1);
	glProgramUniform1f(renderingProgram, mShi1Loc, matShi1);

	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, amb2Loc, 1, lightAmbient2);
	glProgramUniform4fv(renderingProgram, diff2Loc, 1, lightDiffuse2);
	glProgramUniform4fv(renderingProgram, spec2Loc, 1, lightSpecular2);
	glProgramUniform3fv(renderingProgram, pos2Loc, 1, lightPos2);
	glProgramUniform4fv(renderingProgram, mAmb2Loc, 1, matAmb2);
	glProgramUniform4fv(renderingProgram, mDiff2Loc, 1, matDif2);
	glProgramUniform4fv(renderingProgram, mSpec2Loc, 1, matSpe2);
	glProgramUniform1f(renderingProgram, mShi2Loc, matShi2);
}

void installLights(glm::mat4 vMatrix) {
	// convert light’s position to view space, and save it in a float array
	lightPos1V = glm::vec3(vMatrix * glm::vec4(currentLightPos1, 1.0));
	lightPos2V = glm::vec3(vMatrix * glm::vec4(currentLightPos2, 1.0));
	lightPos1[0] = lightPos1V.x;
	lightPos1[1] = lightPos1V.y;
	lightPos1[2] = lightPos1V.z;
	lightPos2[0] = lightPos2V.x;
	lightPos2[1] = lightPos2V.y;
	lightPos2[2] = lightPos2V.z;
	// get the locations of the light and material fields in the shader
	globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
	amb1Loc = glGetUniformLocation(renderingProgram, "light1.ambient");
	diff1Loc = glGetUniformLocation(renderingProgram, "light1.diffuse");
	spec1Loc = glGetUniformLocation(renderingProgram, "light1.specular");
	pos1Loc = glGetUniformLocation(renderingProgram, "light1.position");
	mAmb1Loc = glGetUniformLocation(renderingProgram, "material1.ambient");
	mDiff1Loc = glGetUniformLocation(renderingProgram, "material1.diffuse");
	mSpec1Loc = glGetUniformLocation(renderingProgram, "material1.specular");
	mShi1Loc = glGetUniformLocation(renderingProgram, "material1.shininess");

	amb2Loc = glGetUniformLocation(renderingProgram, "light2.ambient");
	diff2Loc = glGetUniformLocation(renderingProgram, "light2.diffuse");
	spec2Loc = glGetUniformLocation(renderingProgram, "light2.specular");
	pos2Loc = glGetUniformLocation(renderingProgram, "light2.position");
	mAmb2Loc = glGetUniformLocation(renderingProgram, "material2.ambient");
	mDiff2Loc = glGetUniformLocation(renderingProgram, "material2.diffuse");
	mSpec2Loc = glGetUniformLocation(renderingProgram, "material2.specular");
	mShi2Loc = glGetUniformLocation(renderingProgram, "material2.shininess");
	// set the uniform light and material values in the shader
	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, amb1Loc, 1, lightAmbient1);
	glProgramUniform4fv(renderingProgram, diff1Loc, 1, lightDiffuse1);
	glProgramUniform4fv(renderingProgram, spec1Loc, 1, lightSpecular1);
	glProgramUniform3fv(renderingProgram, pos1Loc, 1, lightPos1);
	glProgramUniform4fv(renderingProgram, mAmb1Loc, 1, matAmb1);
	glProgramUniform4fv(renderingProgram, mDiff1Loc, 1, matDif1);
	glProgramUniform4fv(renderingProgram, mSpec1Loc, 1, matSpe1);
	glProgramUniform1f(renderingProgram, mShi1Loc, matShi1);

	glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(renderingProgram, amb2Loc, 1, lightAmbient2);
	glProgramUniform4fv(renderingProgram, diff2Loc, 1, lightDiffuse2);
	glProgramUniform4fv(renderingProgram, spec2Loc, 1, lightSpecular2);
	glProgramUniform3fv(renderingProgram, pos2Loc, 1, lightPos2);
	glProgramUniform4fv(renderingProgram, mAmb2Loc, 1, matAmb2);
	glProgramUniform4fv(renderingProgram, mDiff2Loc, 1, matDif2);
	glProgramUniform4fv(renderingProgram, mSpec2Loc, 1, matSpe2);
	glProgramUniform1f(renderingProgram, mShi2Loc, matShi2);
}

void setPlayerCamera() {
	cameraX = playerCoords.x;
	cameraY = playerCoords.y;
	cameraZ = playerCoords.z;
}

void resetCamera() {
	cameraX = 0.0f;
	cameraY = 0.0f;
	cameraZ = 0.0f;
	cameraDist = -6.0f;
}

glm::vec4 getThirdPersonCameraCoords() {
	glm::vec4 newCC = playerCoords;
	glm::vec4 placeholder = newCC;
	glm::mat4 rMat2;

	rMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	rMat *= glm::translate(glm::mat4(1.0f), glm::vec3(newCC.x, newCC.y, newCC.z));
	rMat *= glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotX), glm::vec3(1.0f, 0.0f, 0.0f));
	//rMat = glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotY), glm::vec3(0.0f, 1.0f, 0.0f)) * rMat; //doing it like this gives proper Y value, but rotates around origin
	//rMat *= glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotY), glm::vec3(0.0f, 1.0f, 0.0f)); doing it like this gives orientation around ship, but oscillating y value
	//potential fix: make dummy vec4s that can get the good bits out of each method of rotation, then feed them to the newCC
	rMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, cameraDist));

	newCC = rMat * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	float nCamY = newCC.y;
	setPlayerCamera();
	newCC = playerCoords;

	//JUST USE THIS

	rMat2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	rMat2 *= glm::translate(glm::mat4(1.0f), glm::vec3(newCC.x, newCC.y, newCC.z));
	//rMat2 *= glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotX), glm::vec3(1.0f, 0.0f, 0.0f));
	//rMat2 = glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotY), glm::vec3(0.0f, 1.0f, 0.0f)) * rMat2; //doing it like this gives proper Y value, but rotates around origin
	rMat2 *= glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotY), glm::vec3(0.0f, 1.0f, 0.0f)); //doing it like this gives orientation around ship, but oscillating y value
	rMat2 *= glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotX), glm::vec3(1.0f, 0.0f, 0.0f));
	//potential fix: make dummy vec4s that can get the good bits out of each method of rotation, then feed them to the newCC
	rMat2 *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, cameraDist));
	placeholder = rMat2 * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	//THIS IS THE STUFF ^^^^^^^^^^6

	newCC.y = placeholder.y;
	newCC.x = placeholder.x;
	newCC.z = placeholder.z;

	//newCC = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotX), glm::vec3(1.0f, 0.0f, 0.0f)) * newCC;
	//newCC = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotY), glm::vec3(0.0f, 1.0f, 0.0f)) * newCC;

	return newCC;
}

void display(GLFWwindow* window, double currentTime) {
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	diffTime = currentTime - lastTime;
	lastTime = currentTime;

	cameraRotX = cameraRotX + cameraRotXspeed;
	if (cameraRotX >= 90.0f) {
		cameraRotX = 90.0f;
	}
	else if (cameraRotX <= -90.0f) {
		cameraRotX = -90.0f;
	}
	cameraRotY = cameraRotY + cameraRotYspeed;
	cameraRotZ = cameraRotZ + cameraRotZspeed;

	cameraXdiff = sin(glm::radians(cameraRotY + cameraRotYadd));
	cameraZdiff = cos(glm::radians(cameraRotY + cameraRotYadd));

	cameraX = cameraX + (cameraXspeed * cameraXdiff) * cameraSpeedMultiplier;
	cameraY = cameraY + (cameraYspeed)* cameraSpeedMultiplier;
	cameraZ = cameraZ + (cameraZspeed * cameraZdiff) * cameraSpeedMultiplier;

	//set camera behind player
	cameraRotY = -player.getRotY() - 90.0f;

	//player coords
	player.calculateRotY();
	player.applyMaxCoordAccel();

	if (playerForwardThrust) {
		player.useBackThruster();
	}
	if (playerBackwardThrust) {
		player.useFrontThruster();
	}
	if (playerCWthrust) {
		player.useTurnClockwise();
	}
	if (playerCCWthrust) {
		player.useTurnCounterClockwise();
	}

	player.SpaceShip::move(diffTime);

	playerCoords = player.getCoords();
	//cameraX = playerCoords.x; cameraY = playerCoords.y; cameraZ = -playerCoords.z;

	glUseProgram(renderingProgram);

	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");
	ltLoc = glGetUniformLocation(renderingProgram, "light_type");
	glUniform1i(ltLoc, 0);

	setLightLoc();

	// build perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees

	setPlayerCamera();

	//do rotations around origin first, then move camera to coordinates around player

	glm::vec4 camCoords = glm::vec4(cameraX, cameraY, cameraZ, 1.0f);

	/*vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-camCoords.x, -camCoords.y, -camCoords.z));
	vMat *= glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotX), glm::vec3(1.0f, 0.0f, 0.0f));
	vMat *= glm::rotate(glm::mat4(1.0f), glm::radians(-cameraRotY), glm::vec3(0.0f, 1.0f, 0.0f));*/
	camCoords = getThirdPersonCameraCoords();

	vMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	vMat *= glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotX), glm::vec3(1.0f, 0.0f, 0.0f));
	vMat *= glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotY), glm::vec3(0.0f, 1.0f, 0.0f));
	vMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-camCoords.x, -camCoords.y, -camCoords.z));
	//vMat *= glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));

	// Prepare to draw the skybox first. The M matrix places the skybox at the camera location
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(cameraX, cameraY, cameraZ));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(camCoords.x, camCoords.y, camCoords.z));
	// build the MODEL-VIEW matrix
	mvMat = vMat * mMat;

	installSkyboxLights(vMat);
	installLightsNoLoc(vMat);

	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	// set up buffer containing vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[21]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// set up buffer containing texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, vbo[22]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	// activate the skybox texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, skyboxTexture);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW); // cube has CW winding order, but we are viewing its interior
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, 36); // draw the skybox without depth testing
	glEnable(GL_DEPTH_TEST);

	int spawnDemon = rand() % 60;
	//cout << spawnDemon << "num enemies: " << demons.size() << endl;
	if ((spawnDemon == 1) && (demons.size() < 700)) { //bug: demons higher up in list are also erased when one lower is eraed?
		
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY()+90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mMat *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 55.0f));

		glm::vec4 nDemCoords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		nDemCoords = mMat * nDemCoords;
		Demon nDemon(nDemCoords, 20, 5); //vec4 was previosuly player coords
		cout << playerCoords.x << " " << playerCoords.y << " " << playerCoords.z << endl;
		demons.push_back(nDemon);
	}



	for (int ii = 0; ii < lasers.size(); ii++) {
		if ((currentTime - lasers[ii].getSpawnTime()) > 4.0) {
			lasers[ii].killLaser();

		}
		if (lasers[ii].getAliveStatus() == false) {
			lasers.erase(lasers.begin() + ii);
		}
	}

	collisionDetection(currentTime);

	// get the uniform variables for the MV and projection matrices
	//mLoc = glGetUniformLocation(renderingProgram, "m_matrix");
	//vLoc = glGetUniformLocation(renderingProgram, "v_matrix");
	mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
	projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

	// build perspective matrix
	glfwGetFramebufferSize(window, &width, &height);
	aspect = (float)width / (float)height;
	pMat = glm::perspective(1.0f*1.0472f, 0.75f*aspect, 0.1f, 1000.0f); // 1.0472 radians = 60 degrees

	// set up lights based on the current light’s position
	currentLightPos1 = glm::vec3(initialLightLoc1.x, initialLightLoc1.y, initialLightLoc1.z);
	if (windowOpen) {
		currentLightPos2 = glm::vec3(initialLightLoc2.x, initialLightLoc2.y, initialLightLoc2.z);
	}
	else {
		currentLightPos2 = glm::vec3(initialLightLoc1.x, initialLightLoc1.y, initialLightLoc1.z);
	}

	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	matAmb1 = silverAmbient();
	matDif1 = silverDiffuse();
	matSpe1 = silverSpecular();
	matShi1 = silverShininess();
	matShi1 = 2.0f;

	matAmb2 = silverAmbient();
	matDif2 = silverDiffuse();
	matSpe2 = silverSpecular();
	matShi2 = silverShininess();
	matShi2 = 2.0f;

	installLights(vMat);

	//planets

	for (int p = 0; p < planets.size(); p++) {
		//to move center of rotation, change translation AFTER the rotation
		//to move distance from center, change translation BEFORE the rotation
		planetCoords = planets[p].getCoords();
		float planetDist = planets[p].getDist();
		float planetRotYfactor = planets[p].getRotYfactor();
		float planetSize = planets[p].getSize();
		//planetCoords.x += planetDist*0.25f;
		planetCoords = glm::rotate(glm::mat4(1.0f), glm::radians(((float)currentTime))*4.0f*planetRotYfactor, glm::vec3(0.0f, 1.0f, 0.0f)) * planetCoords;
		planetCoords = glm::translate(glm::mat4(1.0f), glm::vec3(planetCoords.x, 0.0f, planetCoords.z)) * planetCoords;
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(planetCoords.x, 0.0f, planetCoords.z));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(planetSize, planetSize, planetSize));
		printMatrix(mMat);
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));

		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glBindBuffer(GL_ARRAY_BUFFER, vbo[15]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[16]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[17]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planets[p].getTexture());
		glDrawArrays(GL_TRIANGLES, 0, mpMarsModel.getNumVertices());
	}

	//player 

	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
	mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY() + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));
	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
	mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY() - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	mvMat = vMat * mMat;
	glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[18]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[19]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[20]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spaceShipTexture);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glDrawArrays(GL_TRIANGLES, 0, 834);
	//glDrawArrays(GL_TRIANGLES, 0, spaceShipModel.getNumVertices());

	invTrMat = glm::transpose(glm::inverse(mvMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, spaceShipThrusterTexture);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDrawArrays(GL_TRIANGLES, 834, spaceShipModel.getNumVertices() - 834);

	// cones for thrusters

	matShi1 = 200.0f;
	matShi2 = 200.0f;
	glProgramUniform1f(renderingProgram, mShi1Loc, matShi1);
	glProgramUniform1f(renderingProgram, mShi2Loc, matShi2);
	glUniform1i(ltLoc, 1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, feltTexture);

	if (playerCWthrust == true) {
		//front left

		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY() - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mMat *= glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, 1.3f));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(0.2f + cos(currentTime*30.0)*.1, 0.1f, 0.1f));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glDrawArrays(GL_TRIANGLES, 0, coneModel.getNumVertices());
		//back right

		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY() + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mMat *= glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, 1.5f));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(0.2f + cos(currentTime*30.0)*.1, 0.1f, 0.1f));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glDrawArrays(GL_TRIANGLES, 0, coneModel.getNumVertices());
	}
	//front right

	if (playerCCWthrust == true) {
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY() + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mMat *= glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, -1.3f));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(0.2f + cos(currentTime*30.0)*.1, 0.1f, 0.1f));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glDrawArrays(GL_TRIANGLES, 0, coneModel.getNumVertices());
		//back left

		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY() - 90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mMat *= glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 0.0f, -1.5f));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(0.2f + cos(currentTime*30.0)*.1, 0.1f, 0.1f));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glDrawArrays(GL_TRIANGLES, 0, coneModel.getNumVertices());
	}
	//back left

	if (playerForwardThrust == true) {
		float pftScaleX = 0.00001*(currentTime - thrustTime);
		if (pftScaleX > 0.2f) {
			pftScaleX = 0.2f + cos(currentTime*30.0)*.05;
		}
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(playerCoords.x, 0.0f, playerCoords.z));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(player.getRotY() - 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		mMat *= glm::translate(glm::mat4(1.0f), glm::vec3(2.2f, 0.16f, 0.0f));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(0.2f+cos(currentTime*30.0)*.1, 0.4f, 0.4f));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glDrawArrays(GL_TRIANGLES, 0, coneModel.getNumVertices());
	}

	//player lasers for testing

	for (int l = 0; l < lasers.size(); l++) {
		lasers[l].move(diffTime);
		laserCoords = lasers[l].getCoords();
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(laserCoords.x, 0.0f, laserCoords.z));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(lasers[l].getRotY()), glm::vec3(0.0f, 1.0f, 0.0f));
		//mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 0.25f));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glBindBuffer(GL_ARRAY_BUFFER, vbo[26]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[27]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[28]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, laserBaseTexture);
		glDrawArrays(GL_TRIANGLES, 0, LaserModel.getNumVertices());
	}
	matShi1 = silverShininess();
	matShi2 = silverShininess();
	glProgramUniform1f(renderingProgram, mShi1Loc, matShi1);
	glProgramUniform1f(renderingProgram, mShi2Loc, matShi2);
	glUniform1i(ltLoc, 0);

	//demons
	for (int i = 0; i < demons.size(); i++) {
		if (demons[i].getAliveStatus() == false) {
			demons.erase(demons.begin() + i);
		}
	}
	for (int i = 0; i < demons.size(); i++) {
		demons[i].move(diffTime, playerCoords);
		demonCoords = demons[i].getCoords(); 
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(demonCoords.x, 0.0f, demonCoords.z));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
		mMat *= glm::rotate(glm::mat4(1.0f), glm::radians(demons[i].getRotY()), glm::vec3(0.0f, 1.0f, 0.0f));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));
		glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

		glBindBuffer(GL_ARRAY_BUFFER, vbo[23]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[24]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[25]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, UFOTexture);
		glDrawArrays(GL_TRIANGLES, 0, UFOModel.getNumVertices());
	}

	matAmb1 = bronzeAmbient();
	matDif1 = bronzeDiffuse();
	matSpe1 = bronzeSpecular();
	matShi1 = bronzeShininess();

	matAmb2 = bronzeAmbient();
	matDif2 = bronzeDiffuse();
	matSpe2 = bronzeSpecular();
	matShi2 = bronzeShininess();

	//instancing

	glUseProgram(instancingProgram);

	globalAmbLoc = glGetUniformLocation(instancingProgram, "globalAmbient");
	amb1Loc = glGetUniformLocation(instancingProgram, "light1.ambient");
	diff1Loc = glGetUniformLocation(instancingProgram, "light1.diffuse");
	spec1Loc = glGetUniformLocation(instancingProgram, "light1.specular");
	pos1Loc = glGetUniformLocation(instancingProgram, "light1.position");
	mAmb1Loc = glGetUniformLocation(instancingProgram, "material1.ambient");
	mDiff1Loc = glGetUniformLocation(instancingProgram, "material1.diffuse");
	mSpec1Loc = glGetUniformLocation(instancingProgram, "material1.specular");
	mShi1Loc = glGetUniformLocation(instancingProgram, "material1.shininess");

	amb2Loc = glGetUniformLocation(instancingProgram, "light2.ambient");
	diff2Loc = glGetUniformLocation(instancingProgram, "light2.diffuse");
	spec2Loc = glGetUniformLocation(instancingProgram, "light2.specular");
	pos2Loc = glGetUniformLocation(instancingProgram, "light2.position");
	mAmb2Loc = glGetUniformLocation(instancingProgram, "material2.ambient");
	mDiff2Loc = glGetUniformLocation(instancingProgram, "material2.diffuse");
	mSpec2Loc = glGetUniformLocation(instancingProgram, "material2.specular");
	mShi2Loc = glGetUniformLocation(instancingProgram, "material2.shininess");

	glProgramUniform4fv(instancingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(instancingProgram, amb1Loc, 1, lightAmbient1);
	glProgramUniform4fv(instancingProgram, diff1Loc, 1, lightDiffuse1);
	glProgramUniform4fv(instancingProgram, spec1Loc, 1, lightSpecular1);
	glProgramUniform3fv(instancingProgram, pos1Loc, 1, lightPos1);
	glProgramUniform4fv(instancingProgram, mAmb1Loc, 1, matAmb1);
	glProgramUniform4fv(instancingProgram, mDiff1Loc, 1, matDif1);
	glProgramUniform4fv(instancingProgram, mSpec1Loc, 1, matSpe1);
	glProgramUniform1f(instancingProgram, mShi1Loc, matShi1);

	glProgramUniform4fv(instancingProgram, globalAmbLoc, 1, globalAmbient);
	glProgramUniform4fv(instancingProgram, amb2Loc, 1, lightAmbient2);
	glProgramUniform4fv(instancingProgram, diff2Loc, 1, lightDiffuse2);
	glProgramUniform4fv(instancingProgram, spec2Loc, 1, lightSpecular2);
	glProgramUniform3fv(instancingProgram, pos2Loc, 1, lightPos2);
	glProgramUniform4fv(instancingProgram, mAmb2Loc, 1, matAmb2);
	glProgramUniform4fv(instancingProgram, mDiff2Loc, 1, matDif2);
	glProgramUniform4fv(instancingProgram, mSpec2Loc, 1, matSpe2);
	glProgramUniform1f(instancingProgram, mShi2Loc, matShi2);

	mLoc = glGetUniformLocation(instancingProgram, "m_matrix");
	vLoc = glGetUniformLocation(instancingProgram, "v_matrix");
	projLoc = glGetUniformLocation(instancingProgram, "proj_matrix");
	nLoc = glGetUniformLocation(instancingProgram, "norm_matrix");
	tfLoc = glGetUniformLocation(instancingProgram, "tf");
	mtLoc = glGetUniformLocation(instancingProgram, "modeltype");

	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(25.1f, 25.1f, 25.1f));
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(mMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniform1f(tfLoc, (float)currentTime);
	glUniform1i(mtLoc, 1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[15]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[16]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[17]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lavaTexture);
	glDrawArraysInstanced(GL_TRIANGLES, 0, mpMarsModel.getNumVertices(), 30);


	for (int b = 0; b < bursts.size(); b++) {
		float scale;
		if (bursts[b].getID() == 2) {
			scale = 0.1f;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, laserBaseTexture);
		}
		else {
			scale = 2.1f;
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, lavaTexture);
		}

		glm::vec4 nb = bursts[b].getCoords();
		float nbSpawn = (float)(currentTime - bursts[b].getSpawnTime());
		mMat = glm::translate(glm::mat4(1.0f), glm::vec3(nb.x, 0.0f, nb.z));
		mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, scale));
		mvMat = vMat * mMat;
		invTrMat = glm::transpose(glm::inverse(mvMat));

		glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(mMat));
		glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
		glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
		glUniform1f(tfLoc, nbSpawn);
		glUniform1i(mtLoc, bursts[b].getID());

		glDrawArraysInstanced(GL_TRIANGLES, 0, mpMarsModel.getNumVertices(), bursts[b].getNumInst());

		if (nbSpawn > 2.0f) {
			bursts.erase(bursts.begin() + b);
			b--;
		}
		
	}
	//for explosions, consider a for loop that draws both for the first half of the dying sequence, 
	//at which point it stops drawing the enemy, begins to shrink, then after a full cycle, removes enemy from vector
	//requires the tracking of time since death to work for both.

	//consider a spawning system for enemies in which they come from within a planet but their model starts out
	//as really tiny, giving the illusion of distance, until they are full size. 

	mMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 5.0f));
	mMat *= glm::scale(glm::mat4(1.0f), glm::vec3(2.1f, 2.1f, 2.1f));
	mvMat = vMat * mMat;
	invTrMat = glm::transpose(glm::inverse(mvMat));

	glUniformMatrix4fv(mLoc, 1, GL_FALSE, glm::value_ptr(mMat));
	glUniformMatrix4fv(vLoc, 1, GL_FALSE, glm::value_ptr(vMat));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));
	glUniform1f(tfLoc, (float)currentTime);
	glUniform1i(mtLoc, 3);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lavaTexture);
	glDrawArraysInstanced(GL_TRIANGLES, 0, mpMarsModel.getNumVertices(), 12);
}

void keyCallbackFunc(GLFWwindow* window, int key, int scanmode, int action, int mod) {
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
		cameraRotXspeed = cameraRotXspeed + 2.0f;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
		cameraRotXspeed = cameraRotXspeed - 2.0f;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		cameraRotXspeed = cameraRotXspeed - 2.0f;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
		cameraRotXspeed = cameraRotXspeed + 2.0f;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		cameraRotYspeed = cameraRotYspeed + 2.0f;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
		cameraRotYspeed = cameraRotYspeed - 2.0f;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		cameraRotYspeed = cameraRotYspeed - 2.0f;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
		cameraRotYspeed = cameraRotYspeed + 2.0f;
	}
	else if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		cameraZspeed = cameraZspeed - 0.1f;
		cameraXspeed = cameraXspeed - 0.1f;
		cameraDist = cameraDist - 1.0f;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
		cameraZspeed = cameraZspeed + 0.1f;
		cameraXspeed = cameraXspeed + 0.1f;
	}
	else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		cameraZspeed = cameraZspeed + 0.1f;
		cameraXspeed = cameraXspeed + 0.1f;
		cameraDist = cameraDist + 1.0f;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
		cameraZspeed = cameraZspeed - 0.1f;
		cameraXspeed = cameraXspeed - 0.1f;
	}
	else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		cameraXspeed = cameraXspeed - 0.1f;
		cameraZspeed = cameraZspeed - 0.1f;
		cameraRotYadd = 90.0f;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
		cameraXspeed = cameraXspeed + 0.1f;
		cameraZspeed = cameraZspeed + 0.1f;
		cameraRotYadd = 0.0f;
	}
	else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		cameraXspeed = cameraXspeed + 0.1f;
		cameraZspeed = cameraZspeed + 0.1f;
		cameraRotYadd = 90.0f;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
		cameraXspeed = cameraXspeed - 0.1f;
		cameraZspeed = cameraZspeed - 0.1f;
		cameraRotYadd = 0.0f;
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		cameraYspeed = cameraYspeed + 0.1f;
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
		cameraYspeed = cameraYspeed - 0.1f;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		cameraYspeed = cameraYspeed - 0.1f;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
		cameraYspeed = cameraYspeed + 0.1f;
	}
	else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
		cameraSpeedMultiplier = 2.0f;
	}
	else if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE) {
		cameraSpeedMultiplier = 1.0f;
	}
	else if (key == GLFW_KEY_U && action == GLFW_PRESS) {
		playerForwardThrust = true;
		thrustTime = glfwGetTime();
	}
	else if (key == GLFW_KEY_U && action == GLFW_RELEASE) {
		playerForwardThrust = false;
		player.setAccelToZero();
	}
	else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		playerCCWthrust = true;
	}
	else if (key == GLFW_KEY_H && action == GLFW_RELEASE) {
		playerCCWthrust = false;
	}
	else if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		playerBackwardThrust = true;
	}
	else if (key == GLFW_KEY_J && action == GLFW_RELEASE) {
		playerBackwardThrust = false;
		player.setAccelToZero();
	}
	else if (key == GLFW_KEY_K && action == GLFW_PRESS) {
		playerCWthrust = true;
	}
	else if (key == GLFW_KEY_K && action == GLFW_RELEASE) {
		playerCWthrust = false;
	}
	else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		double ct = glfwGetTime();
		Laser nLaser(playerCoords, player.getRotY(), 1.0f, 5, ct, diffTime);
		lasers.push_back(nLaser);
	}
	else if (key == GLFW_KEY_L && action == GLFW_RELEASE) {
		if (windowOpen == true) { windowOpen = false; }
		else { windowOpen = true; }
	}
	else {
		cameraRotY = cameraRotY;
		cameraRotX = cameraRotX;
	}

}

int main(void) { // main() is unchanged from before
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(1400, 800, "Assignment 6", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);
	init(window);
	while (!glfwWindowShouldClose(window)) {
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwSetKeyCallback(window, &keyCallbackFunc);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}