#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <glm\glm\glm.hpp>
#include <glm\glm\gtc\type_ptr.hpp>
#include <glm\glm\gtc\matrix_transform.hpp>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stack>
#include <vector>
#include <SOIL2/soil2.h>
#include "Planet.h"

Planet::Planet() {
	coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	mass = 10.0f;
	size = 1;
	distFromCenter = 0;
}

Planet::Planet(glm::vec4 c, float m, float s, float d, float ryf) {
	coords = c;
	mass = m;
	size = s;
	distFromCenter = d;
	rotYfactor = ryf;
}
glm::vec4 Planet::getCoords() {
	return coords;
}
void Planet::setCoords(glm::vec4 c) {
	coords = c;
}
float Planet::getSize() {
	return size;
}
void Planet::setSize(float s) {
	size = s;
}
float Planet::getDist() {
	return distFromCenter;
}
void Planet::setDist(float d) {
	distFromCenter = d;
}
float Planet::getRotYfactor() {
	return rotYfactor;
}
void Planet::setRotYfactor(float r) {
	rotYfactor = r;
}

GLuint Planet::getTexture()
{
	return texture;
}

void Planet::setTexture(GLuint t)
{
	texture = t;
}
