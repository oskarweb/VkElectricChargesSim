#pragma once

#include <format>

#include "VulkanRenderer.h"
#include "Camera.h"
#include "Input.h"
#include "Models.h"
#include "Particle.h"

class Simulation
{
public:
	Simulation(VulkanRenderer& rendererHandle) : 
		m_rendererHandle(rendererHandle),
		m_camera(Camera(glm::vec3(2.0f, 2.0f, 1.0f), 0.0f, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)))
	{
	}

	void run();

private:
	void calculateParticlePositions();
	void displayMainCtrlWindow();
	void displayParticleListWindow();
	void displayParticleAddWindow();
	void addParticle(Particle&& particle);
	void removeParticle(std::vector<Particle>::iterator it);
	void restartSimulation();
	void resetAll();
	void initWindow();
	static inline std::string particleHeaderText(const Particle& particle);
	static inline void displayUnitSelector(const std::string& unit, int& prefixIdx);

	std::vector<Particle> m_particles;

	GLFWwindow* m_window = nullptr;
	VulkanRenderer& m_rendererHandle;
	Camera m_camera;

	bool m_simulateFromPrecalculatedSteps = true;
	bool m_paused = true;
	double m_startTime = 0.0;
	double m_elapsedTime = 0.0;
	double m_simulationTime = 0.0;

	Types::ImGuiWindowInfo m_mainCtrlWindowInfo = { MAIN_CTRL_WINDOW_MIN_SIZE, ImVec2(0, 0) };
	Types::ImGuiWindowInfo m_particleListWindowInfo = { PARTICLE_LIST_WINDOW_MIN_SIZE, ImVec2(0, 0) };
	Types::ImGuiWindowInfo m_particleAddWindowInfo = { PARTICLE_ADD_WINDOW_MIN_SIZE, ImVec2(0, 0) };

	inline static constexpr double PARTICLE_TIME_STEP_S = 0.0001; // seconds

	inline static float WINDOWS_BG_ALPHA = 0.50f;
	inline static constexpr ImVec2 MAIN_CTRL_WINDOW_MIN_SIZE = ImVec2(350, 200);
	inline static constexpr ImVec2 PARTICLE_LIST_WINDOW_MIN_SIZE = ImVec2(600, 100);
	inline static constexpr ImVec2 PARTICLE_ADD_WINDOW_MIN_SIZE = ImVec2(280, 200);
	inline static constexpr double SLIDER_MIN_AFFECTING_FORCE = -2.0;
	inline static constexpr double SLIDER_MAX_AFFECTING_FORCE = 2.0;
	inline static constexpr double SLIDER_MIN_POS = 0.0;
	inline static constexpr double SLIDER_MAX_POS = 10.0;
	inline static constexpr double DEFAULT_PARTICLE_MASS = 1.0;
	inline static constexpr double DEFAULT_PARTICLE_CHARGE = 0.01;
	inline static constexpr bool DEFAULT_PARTICLE_MOVABLE = true;
	inline static constexpr Types::Vec3d DEFAULT_PARTICLE_POS = Types::Vec3d(0.0);
	inline static const char* UNIT_PREFIXES[] = { "none", "n", "m", "k" }; \

	struct ParticleConfig
	{
		double charge = DEFAULT_PARTICLE_CHARGE;
		double mass = DEFAULT_PARTICLE_MASS;
		bool movable = DEFAULT_PARTICLE_MOVABLE;
		Types::Vec3d pos = DEFAULT_PARTICLE_POS;
	};

	inline static const ParticleConfig PARTICLE_PRESET1[] = {
		{+0.00'1, 1.0, true, Types::Vec3d(+5.0,+5.0,+5.0) },
		{-0.00'1, 1.0, true, Types::Vec3d(-5.0,-5.0,-5.0) },
		{-0.00'1, 1.0, true, Types::Vec3d(+5.0,+0.0,-5.0) },
		{+0.00'1, 1.0, true, Types::Vec3d(-5.0,+0.0,+5.0) },
	};

	inline static const ParticleConfig PARTICLE_PRESET2[] = {
		{+0.00'1, 1.0, true, Types::Vec3d(-5.0) },
		{-0.00'1, 1.0, true, Types::Vec3d(+5.0) },
	};

	inline static const ParticleConfig PARTICLE_PRESET3[] = {
		{+0.00'1, 1.0, true, Types::Vec3d(+5.0,+5.0,+5.0) },
		{-0.00'1, 1.0, true, Types::Vec3d(-5.0,-5.0,-5.0) },
		{-0.00'1, 1.0, true, Types::Vec3d(+5.0,+0.0,-5.0) },
		{+0.00'1, 1.0, true, Types::Vec3d(-5.0,-0.0,+5.0) },
	};
};

std::string Simulation::particleHeaderText(const Particle& particle)
{
	return std::format("Particle {}", particle.getId());
}