#pragma once

class Demon {
private:
	glm::vec4 coords;
	int health;
	int damage;
	float speedX;
	float accelX;
	float speedZ;
	float accelZ;
	float rotY;
	int atkStatus;
	bool alive;
public: 
	Demon();
	Demon(glm::vec4 c, int h, int dmg);
	glm::vec4 getCoords();
	void Demon::setCoords(glm::vec4 c);
	int getDamage();
	float getSpeedX();
	void Demon::setSpeedX(float sX);
	float getSpeedZ();
	void Demon::setSpeedZ(float sZ);
	float getRotY();
	void Demon::move(double currentTime, glm::vec4 playerCoords);
	void takeDamage(int dmg);
	void Demon::killDemon();
	bool getAliveStatus();
};