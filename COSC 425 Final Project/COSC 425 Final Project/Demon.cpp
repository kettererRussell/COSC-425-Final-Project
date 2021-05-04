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
#include "Demon.h"

#include "C:\\Users\\russk\\Documents\\Visual Studio 2017\\Projects\\COSC 425\\COSC 425 Book Program 7_2\\COSC 425 Book Program 7_2\\ImportedModel.h"

using namespace std;

Demon::Demon() {
	coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	health = 200;
	damage = 5;
	speedX = 0.0f;
	accelX = 0.0f;
	speedZ = 0.0f;
	accelZ = 0.0f;
	rotY = 90.0f;
	//add a time since death variable
}
Demon::Demon(glm::vec4 c, int h, int dmg) {
	coords = c;
	health = h;
	damage = dmg;
	speedX = 0.0f;
	accelX = 2.0f;
	speedZ = 0.0f;
	accelZ = 2.0f;
	rotY = 90.0f;
	atkStatus = 0;
	alive = true;
}

glm::vec4 Demon::getCoords()
{
	return coords;
}
void Demon::setCoords(glm::vec4 c) {
	coords = c;
}
int Demon::getDamage() {
	return damage;
}
float Demon::getSpeedX() {
	return speedX;
}
void Demon::setSpeedX(float sX) {
	speedX = sX;
}
float Demon::getSpeedZ() {
	return speedZ;
}
void Demon::setSpeedZ(float sZ) {
	speedZ = sZ;
}
float Demon::getRotY() {
	return rotY;
}

void Demon::move(double time, glm::vec4 playerCoords) {
	if (playerCoords.x == 0.0f) {
		playerCoords.x = 0.25f;
	}
	if (playerCoords.z == 0.0f) {
		playerCoords.z = 0.25f;
	}

	float distX = coords.x - playerCoords.x;
	float distZ = coords.z - playerCoords.z;

	rotY = -atan(((double)distZ / (double)distX));
	if (distX <= 0) {
		rotY = rotY + 3.1415926f;
	}

	accelX = -(0.0006f*(cos(rotY)));
	accelZ = (0.0006f*(sin(rotY)));

	speedX = speedX + accelX;
	speedZ = speedZ + accelZ;

	speedX = std::min(speedX, 0.3f);
	speedZ = std::min(speedZ, 0.3f);
	speedX = std::max(speedX, -0.3f);
	speedZ = std::max(speedZ, -0.3f);

	//if distX is positive, rotY needs to have +180 degress added to it
	//for testing; remove later 
	//speedX = -(0.002f*(cos(rotY)));
	//speedZ = (0.002f*(sin(rotY)));

	rotY = glm::degrees(rotY);

	coords.x = coords.x + (100 * (float)time * speedX);
	coords.z = coords.z + (100 * (float)time * speedZ);
	//std::cout << "distX: " << distX << " distZ: " << distZ << " Roty: " << rotY << " X: " << coords.x << " Z: " << coords.z << endl;
}

void Demon::takeDamage(int dmg) {
	health = health - dmg;
	if (health <= 0) {
		alive = false;
	}
}

void Demon::killDemon() {
	alive = false;
}

bool Demon::getAliveStatus()
{
	return alive;
}
