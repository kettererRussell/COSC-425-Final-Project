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
#include "SpaceShip.h"

#include "C:\\Users\\russk\\Documents\\Visual Studio 2017\\Projects\\COSC 425\\COSC 425 Book Program 7_2\\COSC 425 Book Program 7_2\\ImportedModel.h"

using namespace std;

SpaceShip::SpaceShip() {
	coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	health = 200;
	speedX = 0.0f;
	accelX = 0.0f;
	speedZ = 0.0f;
	accelZ = 0.0f;
	rotY = 90.0f;
	speedRotY = 0.0f;
}
SpaceShip::SpaceShip(glm::vec4 c, int h, float sX, float aX, float sZ, float aZ, float rY, float aY)
{
	coords = c;
	health = h;
	speedX = sX;
	accelX = aX;
	speedZ = sZ;
	accelZ = aZ;
	rotY = rY;
	speedRotY = aY;
}
glm::vec4 SpaceShip::getCoords()
{
	return coords;
}
void SpaceShip::setCoords(glm::vec4 c) {
	coords = c;
}
float SpaceShip::getSpeedX() {
	return speedX;
}
void SpaceShip::setSpeedX(float sX){
	speedX = sX;
}
float SpaceShip::getSpeedZ() {
	return speedZ;
}
void SpaceShip::setSpeedZ(float sZ) {
	speedZ = sZ;
}
float SpaceShip::getRotY() {
	return rotY;
}
void SpaceShip::setRotY(float rY) {
	rotY = rY;
}
void SpaceShip::calculateRotY() {
	rotY = rotY + speedRotY;
}
void SpaceShip::applyMaxCoordAccel() {
	accelX = std::min(accelX, 0.0002f);
	accelZ = std::min(accelZ, 0.0002f);
	accelX = std::max(accelX, -0.0002f);
	accelZ = std::max(accelZ, -0.0002f);

	speedX = std::min(speedX, 0.2f);
	speedZ = std::min(speedZ, 0.2f);
	speedX = std::max(speedX, -0.2f);
	speedZ = std::max(speedZ, -0.2f);
}
void SpaceShip::useTurnClockwise() {
	//speedRotY = speedRotY - 0.2f;
	rotY = rotY - 2.2f;
	speedRotY = std::max(speedRotY, -0.8f);
}
void SpaceShip::useTurnCounterClockwise() {
	//speedRotY = speedRotY + 0.2f;
	rotY = rotY + 2.2f;
	speedRotY = std::min(speedRotY, 0.8f);
}
void SpaceShip::useBackThruster() {
	//accelX = accelX + (0.0001f*(cos(glm::radians(rotY))));
	//accelZ = accelZ + (0.0001f*(sin(glm::radians(rotY))));
	accelX = -(0.0006f*(cos(glm::radians(rotY))));
	accelZ = (0.0006f*(sin(glm::radians(rotY))));
}
void SpaceShip::useFrontThruster() {
	//accelX = accelX - (0.0001f*(cos(glm::radians(rotY))));
	//accelZ = accelZ - (0.0001f*(sin(glm::radians(rotY))));
	accelX = (0.0006f*(cos(glm::radians(rotY))));
	accelZ = -(0.0006f*(sin(glm::radians(rotY))));
}
void SpaceShip::setAccelToZero() {
	accelX = 0.0f;
	accelZ = 0.0f;
}
void SpaceShip::move(double time) {
	speedX = speedX + accelX;
	speedZ = speedZ + accelZ;
	
	coords.x = coords.x + (100*(float)time * speedX);
	coords.z = coords.z + (100*(float)time * speedZ);
}
void SpaceShip::takeDamage(int dmg) {
	health = health - dmg;
	if (health <= 0) {
		health = 0; // add alive status 
	}
}
