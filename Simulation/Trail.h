#pragma once

#include "Node.h"

class Trail : public Node
{
	int idx = 0;
	glm::vec3 m_lastPos;
	glm::vec3 m_initialPos;

public:
	Trail(glm::vec3 startPos) : m_initialPos(startPos), m_lastPos(startPos) {}
	void setInitialPos(glm::vec3 pos) { m_initialPos = pos; }
	void setLastPos(glm::vec3 pos) { m_lastPos = pos; }
	void addSection(glm::vec3 from, glm::vec3 to);
	void addSection(glm::vec3 to);
	void cleanup() override;
};