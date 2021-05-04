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
#include "BurstParticle.h" 

BurstParticle::BurstParticle()
{
	coords = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	double spawnTime = 0.0;
	bool alive = true;
}

BurstParticle::BurstParticle(glm::vec4 c, double st, int numI, int id)
{
	coords = c;
	spawnTime = st;
	alive = true;
	numInst = numI;
	ID = id;
}

glm::vec4 BurstParticle::getCoords()
{
	return coords;
}

void BurstParticle::setCoords(glm::vec4 c)
{
	coords = c;
}

double BurstParticle::getSpawnTime()
{
	return spawnTime;
}

bool BurstParticle::getAliveStatus()
{
	return alive;
}

void BurstParticle::killBurstParticle()
{
	alive = false;
}

int BurstParticle::getNumInst()
{
	return numInst;
}

void BurstParticle::setNumInst(int ni)
{
	numInst = ni;
}

int BurstParticle::getID()
{
	return ID;
}

void BurstParticle::setID(int id)
{
	ID = id;
}
