#pragma once

#include <glm/glm.hpp>

#include <optional>

#include "RendererStructs.h"

class Particle 
{
public:
	Particle() : m_charge(0.0f), m_mass(0.0f), m_affectingForce(0.0f), m_movable(true) {}
	Particle(
		float charge,
		float mass,
		float affectingForce,
		bool movable
	) : m_charge(charge), m_mass(mass), m_affectingForce(affectingForce), m_movable(movable) 
	{

	}

	Particle(
		float charge, 
		float mass, 
		float affectingForce,
		bool movable, 
		Renderable model
	) : m_charge(charge), m_mass(mass), m_affectingForce(affectingForce), m_movable(movable)
	{
		*m_model = std::move(model);
	}

private:
	float m_charge;
	float m_mass;
	float m_affectingForce;
	bool m_movable;
	
	std::optional<Renderable> m_model;
};