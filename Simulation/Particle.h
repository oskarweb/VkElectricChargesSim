#pragma once

#include <glm/glm.hpp>

#include <optional>

#include "RendererStructs.h"
#include "Models.h"
#include "Node.h"

class Particle : public Node
{
	inline static const std::string& F_VECTOR_MODEL_NAME = "forceVector";
	inline static const std::string& P_MODEL_NAME = "particle";
public:
	Particle() : 
		m_charge(0.0f), 
		m_mass(0.0f), 
		m_movable(true),
		m_affectingForce(0.0f), 
		m_pos(glm::vec3(0.0f))
	{
		uploadModel(P_MODEL_NAME, ParticleModel(m_pos));
		uploadModel(F_VECTOR_MODEL_NAME, VectorArrowModel(m_pos, m_affectingForce));
	}

	Particle(
		float charge,
		float mass,
		bool movable,
		glm::vec3 affectingForce,
		glm::vec3 pos
	) : 
		m_charge(charge), 
		m_mass(mass),
		m_affectingForce(affectingForce),
		m_pos(pos),
		m_movable(movable)
	{
		uploadModel(P_MODEL_NAME, ParticleModel(m_pos));
		uploadModel(F_VECTOR_MODEL_NAME, VectorArrowModel(m_pos, m_affectingForce));
	}

	void update()
	{

	}

private:
	float m_charge;
	float m_mass;
	bool m_movable;
	glm::vec3 m_affectingForce;
	glm::vec3 m_pos;
};