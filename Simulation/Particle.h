#pragma once

#include <glm/glm.hpp>

#include <optional>
#include <cmath>
#include <memory>
#include <thread>
#include <mutex>

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

	Particle(const Particle&) = delete;
	Particle& operator=(const Particle&) = delete;
	Particle(Particle&&) noexcept = default;
	Particle& operator=(Particle&&) noexcept = default;

	void update();
	void update(Types::Vec3d affectingForce, double time);
	void update(double time);
	void pushState(uint32_t idx);
	void pushState(uint32_t& idx, Types::Vec3d& force, Types::Vec3d& acceleration, Types::Vec3d& velocity, Types::Vec3d& pos);
	void setInitialState();
	inline void clearStates() { m_states.clear(); }
	bool updateFromPrecalcPos(uint32_t idx);
	void cleanup() override;

	inline Types::Vec3d getCoulombForce(Particle& other) const
	{
		// F = k * |q1 * q2| / r^2
		Types::Vec3d distanceV = m_pos - other.getPos();
		return COULOMB_CONSTANT * m_charge * other.getCharge() * distanceV.normalized(Constants::SOFTENING_CONSTANT) / distanceV.length2(Constants::SOFTENING_CONSTANT * 1e7);
	}

	inline Types::Vec3d getCoulombForce(uint32_t stateIdx, Particle& other)
	{
		// F = k * |q1 * q2| / r^2
		Types::Vec3d distanceV = m_states[stateIdx].pos - other.statesData()[stateIdx].pos;
		return COULOMB_CONSTANT * m_charge * other.getCharge() * distanceV.normalized(Constants::SOFTENING_CONSTANT) / distanceV.length2(Constants::SOFTENING_CONSTANT * 1e7);
	}

	std::mutex& mutexData()
	{
		return *m_statesMutex; 
	}

	std::map<uint32_t, State>& statesData()
	{
		return m_states;
	}

	uint32_t			getBufferedStepCount() const { return static_cast<uint32_t>(m_states.size()); }	
	void				setMaxStep(uint32_t maxStep) { *m_maxStep = maxStep; }
	const uint32_t&     getMaxStep() const { return *m_maxStep; }
	State*				initialStateData() { return m_initialState.get(); }
	State&				getInitialState() { return *m_initialState; }
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

	std::unique_ptr<State> m_initialState;
	std::map<uint32_t, State> m_states;

	std::unique_ptr<std::mutex> m_statesMutex;

	std::unique_ptr<std::atomic<uint32_t>> m_maxStep = 0;

	uint64_t m_id = 0;
	inline static uint64_t nextId = 0;
};