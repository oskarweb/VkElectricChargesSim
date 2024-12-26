#include "Particle.h"

Particle::Particle() :
	m_charge(0.0),
	m_mass(0.0),
	m_affectingForce(Types::Vec3d(0.0)),
	m_acceleration(Types::Vec3d(0.0)),
	m_velocity(Types::Vec3d(0.0)),
	m_pos(Types::Vec3d(0.0)),
	m_movable(true),
	m_id(nextId++)
{
	uploadModel(P_MODEL_NAME, std::make_unique<ParticleModel>(static_cast<glm::vec3>(m_pos)));
	uploadModel(F_VECTOR_MODEL_NAME, std::make_unique<VectorArrowModel>(static_cast<glm::vec3>(m_pos), static_cast<glm::vec3>(m_affectingForce), glm::vec3(0.0f)));
	setInitialState();
}

Particle::Particle(
	double charge,
	double mass,
	bool movable,
	Types::Vec3d pos
) :
	m_charge(charge),
	m_mass(mass),
	m_affectingForce(Types::Vec3d(0.0)),
	m_acceleration(Types::Vec3d(0.0)),
	m_velocity(Types::Vec3d(0.0)),
	m_pos(pos),
	m_movable(movable),
	m_id(nextId++)
{
	uploadModel(P_MODEL_NAME, std::make_unique<ParticleModel>(static_cast<glm::vec3>(m_pos)));
	uploadModel(F_VECTOR_MODEL_NAME, std::make_unique<VectorArrowModel>(static_cast<glm::vec3>(m_pos), static_cast<glm::vec3>(m_affectingForce), glm::vec3(0.0f)));
	setInitialState();
}

Particle::Particle(
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
	m_velocity(Types::Vec3d(0.0)),
	m_pos(pos),
	m_movable(movable),
	m_id(nextId++)
{
	uploadModel(P_MODEL_NAME, std::make_unique<ParticleModel>(static_cast<glm::vec3>(m_pos)));
	uploadModel(F_VECTOR_MODEL_NAME, std::make_unique<VectorArrowModel>(static_cast<glm::vec3>(m_pos), static_cast<glm::vec3>(m_affectingForce), glm::vec3(0.0f)));
	setInitialState();
}

void Particle::update(Types::Vec3d affectingForce, double time)
{
	if (!m_movable) { return; }
	m_affectingForce = affectingForce;
	m_acceleration = m_affectingForce / m_mass;
	m_velocity += m_acceleration;
	m_pos += m_velocity * time;

	m_models[P_MODEL_NAME]->update(static_cast<glm::vec3>(m_pos));
	m_models[F_VECTOR_MODEL_NAME]->update(
		static_cast<glm::vec3>(m_pos), 
		static_cast<glm::vec3>(m_affectingForce),
		glm::normalize(static_cast<glm::vec3>(m_affectingForce)) * 2.5f
	);
}

void Particle::update()
{
	m_models[P_MODEL_NAME]->update(static_cast<glm::vec3>(m_pos));
	m_models[F_VECTOR_MODEL_NAME]->update(
		static_cast<glm::vec3>(m_pos),
		static_cast<glm::vec3>(m_affectingForce),
		glm::normalize(static_cast<glm::vec3>(m_affectingForce)) * 2.5f
	);
}

void Particle::update(double time)
{
	if (!m_movable) { return; }
	m_acceleration = m_affectingForce / m_mass;
	m_velocity += m_acceleration;
	m_pos += m_velocity * time;

	m_models[P_MODEL_NAME]->update(static_cast<glm::vec3>(m_pos));
	m_models[F_VECTOR_MODEL_NAME]->update(
		static_cast<glm::vec3>(m_pos), 
		static_cast<glm::vec3>(m_affectingForce),
		glm::normalize(static_cast<glm::vec3>(m_affectingForce)) * 2.5f
	);
}

void Particle::pushState()
{
	m_states.emplace_back(State{
		m_affectingForce,
		m_acceleration,
		m_velocity,
		m_pos
	});
}

void Particle::setInitialState()
{
	m_initialState = State{
		m_affectingForce,
		m_acceleration,
		m_velocity,
		m_pos
	};
}

void Particle::updateFromPrecalcPos(uint32_t idx)
{
	if (idx < m_states.size())
	{
		m_affectingForce = m_states[idx].affectingForce;
		m_acceleration = m_states[idx].acceleration;
		m_velocity = m_states[idx].velocity;
		m_pos = m_states[idx].pos;

		m_models[P_MODEL_NAME]->update(static_cast<glm::vec3>(m_states[idx].pos));
		m_models[F_VECTOR_MODEL_NAME]->update(
			static_cast<glm::vec3>(m_states[idx].pos), 
			static_cast<glm::vec3>(m_states[idx].affectingForce),
			glm::normalize(static_cast<glm::vec3>(m_states[idx].affectingForce)) * 2.5f
		);
	}
}

void Particle::cleanup()
{
	Node::cleanup();
	m_states.clear();
}