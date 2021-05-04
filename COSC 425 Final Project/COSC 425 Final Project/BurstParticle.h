#pragma once

class BurstParticle {
private:
	glm::vec4 coords;
	double spawnTime;
	bool alive;
	int numInst;
	int ID;
public:
	BurstParticle();
	BurstParticle(glm::vec4 c, double st, int numI, int id);
	glm::vec4 getCoords();
	void setCoords(glm::vec4 c);
	double getSpawnTime();
	bool getAliveStatus();
	void killBurstParticle();
	int getNumInst();
	void setNumInst(int ni);
	int getID();
	void setID(int id);
};