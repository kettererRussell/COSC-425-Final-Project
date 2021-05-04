#pragma once
class Laser {
private:
	glm::vec4 coords;
	float rotY;
	float speedX;
	float speedZ;
	float size;
	int damage;
	double spawnTime;
	bool alive;
public:
	Laser();
	Laser(glm::vec4 c, float r, float s, int d, double spT, double dt);
	glm::vec4 Laser::getCoords();
	float getRotY();
	float getSize();
	int getDamage();
	double getSpawnTime();
	bool getAliveStatus();
	void move(double time);
	void killLaser();
};