#pragma once

#include <glm/glm.hpp>

#include <optional>

#include "RendererStructs.h"
#include "Models.h"
#include "Node.h"
#include "extras.h"

class Particle : public Node
{
public:
	inline static const std::string& F_VECTOR_MODEL_NAME = "forceVector";
	inline static const std::string& P_MODEL_NAME = "particle";
	inline static constexpr glm::vec3 F_VECTOR_OFFEST = glm::vec3(0.0f, 0.0f, 0.0f);

	Particle() : 
		m_charge(0.0), 
		m_mass(0.0), 
		m_movable(true),
		m_affectingForce(0.0), 
		m_acceleration(0.0),
		m_velocity(0.0),
		m_pos(0.0),
		m_id(nextId++)
	{
		uploadModel(P_MODEL_NAME, std::make_unique<ParticleModel>(static_cast<Types::Vec3d>(m_pos)));
		uploadModel(F_VECTOR_MODEL_NAME, std::make_unique<VectorArrowModel>(static_cast<Types::Vec3d>(m_pos), static_cast<Types::Vec3d>(m_affectingForce), glm::vec3(0.0f)));
	}

	Particle(
		double charge,
		double mass,
		bool movable,
		Types::Vec3d affectingForce,
		Types::Vec3d pos
	) : 
		m_charge(charge), 
		m_mass(mass),
		m_affectingForce(affectingForce),
		m_acceleration(affectingForce / mass),
		m_velocity(0.0),
		m_pos(pos),
		m_movable(movable),
		m_id(nextId++)
	{
		uploadModel(P_MODEL_NAME, std::make_unique<ParticleModel>(static_cast<Types::Vec3d>(m_pos)));
		uploadModel(F_VECTOR_MODEL_NAME, std::make_unique<VectorArrowModel>(static_cast<Types::Vec3d>(m_pos), static_cast<Types::Vec3d>(m_affectingForce), glm::vec3(0.0f)));
		m_models[F_VECTOR_MODEL_NAME]->update(static_cast<Types::Vec3d>(m_pos), static_cast<Types::Vec3d>(m_affectingForce), glm::normalize(static_cast<glm::vec3>(m_affectingForce)) * 5.0f);
	}

	// F = m * a, a = F / m

	void update(Types::Vec3d affectingForce, double time)
	{
		if (!m_movable) { return; }
		m_affectingForce = affectingForce;
		m_acceleration = m_affectingForce / m_mass;
		m_velocity += m_acceleration;
		m_pos += m_velocity * time;

		m_models[P_MODEL_NAME]->update(static_cast<Types::Vec3d>(m_pos));
		m_models[F_VECTOR_MODEL_NAME]->update(static_cast<Types::Vec3d>(m_pos), static_cast<Types::Vec3d>(m_affectingForce), glm::normalize(static_cast<glm::vec3>(m_affectingForce) * 5.0f));
	}

	void update(double time)
	{
		if (!m_movable) { return; }
		m_acceleration = m_affectingForce / m_mass;
		m_velocity += m_acceleration;
		m_pos += m_velocity * time;

		m_models[P_MODEL_NAME]->update(static_cast<Types::Vec3d>(m_pos));
		m_models[F_VECTOR_MODEL_NAME]->update(static_cast<Types::Vec3d>(m_pos), static_cast<Types::Vec3d>(m_affectingForce), glm::normalize(static_cast<glm::vec3>(m_affectingForce)) * 5.0f);
	}

	double&				chargeData() { return m_charge; }
	double&				massData() { return m_mass; }
	Types::Vec3d&		affectingForceData() { return m_affectingForce; }
	Types::Vec3d&		accelerationData() { return m_acceleration; }
	Types::Vec3d&		velocityData() { return m_velocity; }
	Types::Vec3d&		posData() { return m_pos; }
	const double&		getCharge() const { return m_charge; }
	const double&		getMass() const { return m_mass; }
	const Types::Vec3d&	getAffectingForce() const { return m_affectingForce; }
	const Types::Vec3d&	getAcceleration() const { return m_acceleration; }
	const Types::Vec3d&	getVelocity() const { return m_velocity; }
	const Types::Vec3d&	getPos() const { return m_pos; }
	void				setCharge(double charge) { m_charge = charge; }
	void				setMass(double mass) { m_mass = std::clamp(mass, 0.0, 1000.0); };
	void				setAffectingForce(Types::Vec3d affectingForce) { m_affectingForce = affectingForce; }
	void				setAcceleration(Types::Vec3d acceleration) { m_acceleration = acceleration; }
	void				setVelocity(Types::Vec3d velocity) { m_velocity = velocity; }
	void				setPos(Types::Vec3d pos) { m_pos = pos; }

	bool setMovable(bool movable)
	{
		m_movable = movable;
		return m_movable;
	}
	const bool isMovable() const { return m_movable; }
	const uint64_t& getId() const { return m_id; }

private:
	double m_charge;
	double m_mass;
	Types::Vec3d m_affectingForce;
	Types::Vec3d m_acceleration;
	Types::Vec3d m_velocity;
	Types::Vec3d m_pos;
	bool m_movable;

	uint64_t m_id = 0;
	inline static uint64_t nextId = 0;
};