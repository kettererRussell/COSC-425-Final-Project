#pragma once
class Planet {
private:
	glm::vec4 coords;
	float mass;
	float size;
	float distFromCenter;
	float rotYfactor;
	GLuint texture;
public:
	Planet();
	Planet(glm::vec4 c, float m, float s, float d, float ryf);

	glm::vec4 Planet::getCoords();
	void setCoords(glm::vec4 c);
	float getSize();
	void setSize(float s);
	float getDist();
	void setDist(float d);
	float getRotYfactor();
	void setRotYfactor(float r);
	GLuint getTexture();
	void setTexture(GLuint t);
};