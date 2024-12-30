#include "Trail.h"

void Trail::addSection(glm::vec3 from, glm::vec3 to)
{
	uploadModel(std::to_string(idx), std::make_unique<LineModel>(from, to));
	idx++;
}

void Trail::addSection(glm::vec3 to)
{
	uploadModel(std::to_string(idx), std::make_unique<LineModel>(m_lastPos, to));
	idx++;
	m_lastPos = to;
}

void Trail::cleanup()
{
	Node::cleanup();
	m_models.clear();
	m_lastPos = m_initialPos;
	idx = 0;
}