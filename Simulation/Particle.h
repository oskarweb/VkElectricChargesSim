#pragma once

#include <glm/glm.hpp>

#include <optional>
#include <cmath>

#include "RendererStructs.h"
#include "Models.h"
#include "Node.h"
#include "extras.h"

class Particle : public Node
{
public:
	
	struct State
	{
		Types::Vec3d affectingForce;
		Types::Vec3d acceleration;
		Types::Vec3d velocity;
		Types::Vec3d pos;
	};

	Particle();
	Particle(double charge, double mass, bool movable, Types::Vec3d pos);
	Particle(double charge, double mass, bool movable, Types::Vec3d affectingForce, Types::Vec3d pos);

	void update(Types::Vec3d affectingForce, double time);
	void update(double time);
	void pushState();
	void updateFromPrecalcPos(uint32_t idx);
	void cleanup() override;

	inline Types::Vec3d getCoulombForce(Particle& other)
	{
		// F = k * |q1 * q2| / r^2
		return (COULOMB_CONSTANT * m_charge * other.getCharge()) / (m_pos - other.getPos()).length2();
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
	static void resetId() { nextId = 0; }

	inline static constexpr double ELECTRIC_CONSTANT = 8.854187817e-12; // [F / m]
	inline static constexpr double COULOMB_CONSTANT = 8.9875517873681764e9; // 1 / (4 * pi * ELECTRIC_CONSTANT) [N * m^2 / C^2]

	inline static const std::string& F_VECTOR_MODEL_NAME = "forceVector";
	inline static const std::string& P_MODEL_NAME = "particle";
	inline static constexpr glm::vec3 F_VECTOR_OFFEST = glm::vec3(0.0f, 0.0f, 0.0f);
private:
	double m_charge;
	double m_mass;
	Types::Vec3d m_affectingForce;
	Types::Vec3d m_acceleration;
	Types::Vec3d m_velocity;
	Types::Vec3d m_pos;
	bool m_movable;

	std::vector<State> m_states;

	uint64_t m_id = 0;
	inline static uint64_t nextId = 0;
};