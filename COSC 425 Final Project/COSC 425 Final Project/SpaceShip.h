#pragma once
class SpaceShip {
private:
	glm::vec4 coords;
	int health;
	float mass;
	float speedX, accelX;
	float speedZ, accelZ;
	float rotY, speedRotY;
public:
	SpaceShip();
	SpaceShip(glm::vec4 c, int h, float sX, float aX, float sZ, float aZ, float rY, float aY);
	glm::vec4 getCoords();
	void setCoords(glm::vec4 c);

	float getSpeedX();
	void setSpeedX(float sX);
	float getSpeedZ();
	void setSpeedZ(float sZ);

	float getRotY();
	void setRotY(float rY);

	void SpaceShip::calculateRotY();
	void SpaceShip::applyMaxCoordAccel();

	void SpaceShip::useTurnClockwise();
	void SpaceShip::useTurnCounterClockwise();
	void SpaceShip::useBackThruster();
	void SpaceShip::useFrontThruster();
	void SpaceShip::setAccelToZero();
	
	void SpaceShip::move(double time);
	void takeDamage(int dmg);
};