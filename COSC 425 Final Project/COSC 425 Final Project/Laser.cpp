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
#include "Laser.h"

Laser::Laser() {
	coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	rotY = 0.0f;
	speedX = 0.0f;
	speedZ = 0.0f;
	size = 1.0f;
	damage = 5;
	spawnTime = 0.0;
	alive = true;
}

Laser::Laser(glm::vec4 c, float r, float s, int d, double spT, double dt) {
	coords = c;
	rotY = r;
	size = s;
	damage = d;
	spawnTime = spT;
	alive = true;

	speedX = -(0.2f*(cos(glm::radians(rotY)))); //sonsider adding this to player speed
	speedZ = (0.2f*(sin(glm::radians(rotY))));
	//coords.x += (100.0f*(float)dt * speedX);
	//coords.z += (100.0f*(float)dt * speedZ);
	coords.x += (12.0f * speedX);
	coords.z += (12.0f * speedZ);
}

glm::vec4 Laser::getCoords() {
	return coords;
}

float Laser::getRotY()
{
	return rotY;
}

float Laser::getSize()
{
	return size;
}

int Laser::getDamage() {
	return damage;
}

double Laser::getSpawnTime()
{
	return spawnTime;
}

bool Laser::getAliveStatus()
{
	return alive;
}

void Laser::move(double time) {
	coords.x = coords.x + (100*(float)time * speedX);
	coords.z = coords.z + (100*(float)time * speedZ);
}

void Laser::killLaser() {
	alive = false;
}