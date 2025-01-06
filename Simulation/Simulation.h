#pragma once

#include <format>
#include <functional>
#include <thread>
#include <mutex>

#include "VulkanRenderer.h"
#include "Camera.h"
#include "Input.h"
#include "Models.h"
#include "Particle.h"
#include "ChargedCuboid.h"

class Simulation
{
public:
	Simulation(VulkanRenderer& rendererHandle) : 
		m_rendererHandle(rendererHandle),
		m_camera(Camera(glm::vec3(-10.0f, -10.0f, -10.0f), 0.0f, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f))),
		m_threads(NUM_THREADS)
	{
	}

	~Simulation()
	{
		if (m_threadedCalculation)
		{
			for (auto& thread : m_threads)
			{
				if (thread.joinable())
					thread.join();
			}
		}
	}

	void run();

	enum class SimulationMode
	{
		Static = 0,
		PrecalculatedAll,
		Precalculated20Ms,
		Realtime,
	};

	enum class TrailColors
	{
		Red,
		Yellow
	};

private:
	void updateStatic();
	void updateAllPrecalc();
	void update20MsPecalc();
	void updateRealTime();

	Types::Vec3d calcForce(uint32_t stateId, Particle& particle, Types::Vec3d distanceMod = 0.0);

	void calculateSteps(uint32_t startingStep);
	void rk4Step(uint32_t startingStep, Particle& particle);
	void backwardEulerStep(uint32_t stepIdx, Particle& particle);
	
	bool updatePositions();
	void updatePositionsThreaded();
	void calculateParticlePositions(bool all = false);
	void launchParticleThreads();
	void calculateParticlePostionsThreaded(std::stop_token stopToken, uint32_t minIdx, uint32_t maxIdx);
	void calculatePositionsForSingleParticle(Particle* particle);
	void addParticle(Particle&& particle);
	void removeParticle(std::vector<Particle>::iterator& it);
	void startSimulation();
	void restartSimulation();
	void resetAll();
	void initWindow();
	uint32_t getMaxStepsBuffered() { return 1'000'000'000u / static_cast<uint32_t>(sizeof(Particle::State)) / (static_cast<uint32_t>(m_particles.size()) + 1); }
	uint32_t getStepsPer20ms() { return static_cast<uint32_t>(0.02 / m_timeStep); }

	// GUI
	void displayMainCtrlWindow();
	void displayParticleListWindow();
	void displayParticleAddWindow();
	void displayPlotWindow();
	static inline std::string particleHeaderText(const Particle& particle);
	static inline void displayUnitSelector(const std::string& unit, int& prefixIdx);

	//
	// VARIABLES
	//
	
	GLFWwindow* m_window = nullptr;
	VulkanRenderer& m_rendererHandle;
	Camera m_camera;

	SimulationMode m_mode = SimulationMode::Static;
	Types::OdeMethod m_method = Types::OdeMethod::RK4;

	std::vector<Particle> m_particles;
	std::vector<ChargedCuboid> m_cuboids; // Possibly implement charged volume abstract class

	bool m_skipUpdate = true;
	std::atomic<bool> m_paused = true;
	std::atomic<double> m_elapsedTime = 0.0;
	double m_startTime = 0.0;
	double m_simulationTime = DEFAULT_SIMULATION_TIME;
	double m_timeStep = DEFAULT_TIME_STEP;

	std::atomic<uint32_t> m_mutualMaxStep = 0;
	std::atomic<uint32_t> m_maxUsedStep = 0;

	// MULTITHREADING 
	bool m_threadedCalculation = false;
	std::vector<std::jthread> m_threads;
	std::condition_variable m_mutualStepCv;

	bool m_isHung = false;
	std::vector<Particle>::iterator m_hungIt;

	// GUI
	std::vector<Particle>::iterator m_plotSelectedParticle;

	Types::ImGuiWindowInfo m_mainCtrlWindowInfo = { MAIN_CTRL_WINDOW_MIN_SIZE, ImVec2(0, 0) };
	Types::ImGuiWindowInfo m_particleListWindowInfo = { PARTICLE_LIST_WINDOW_MIN_SIZE, ImVec2(0, 0) };
	Types::ImGuiWindowInfo m_particleAddWindowInfo = { PARTICLE_ADD_WINDOW_MIN_SIZE, ImVec2(0, 0) };
	Types::ImGuiWindowInfo m_plotWindowInfo = { PLOT_WINDOW_MIN_SIZE, ImVec2(0, 0) };
	
	// CONSTANTS
	inline static constexpr int BACKWARD_EULER_ITERS = 20;
	inline static constexpr double BACKWARD_EULER_TOLERANCE = 1.0e-3;
	inline static constexpr double DEFAULT_SIMULATION_TIME = 2.0;
	inline static constexpr double DEFAULT_TIME_STEP = 0.0001;
	inline static constexpr uint32_t STEPS_BUFFERED_AT_ONCE = 1000;
	inline static constexpr uint32_t NUM_THREADS = 2;
	inline static constexpr float WINDOWS_BG_ALPHA = 0.50f;
	inline static constexpr ImVec2 MAIN_CTRL_WINDOW_MIN_SIZE = ImVec2(350, 200);
	inline static constexpr ImVec2 PARTICLE_LIST_WINDOW_MIN_SIZE = ImVec2(600, 100);
	inline static constexpr ImVec2 PARTICLE_ADD_WINDOW_MIN_SIZE = ImVec2(280, 200);
	inline static constexpr ImVec2 PLOT_WINDOW_MIN_SIZE = ImVec2(400, 400);
	inline static constexpr double SLIDER_MIN_AFFECTING_FORCE = -2.0;
	inline static constexpr double SLIDER_MAX_AFFECTING_FORCE = 2.0;
	inline static constexpr double SLIDER_MIN_POS = 0.0;
	inline static constexpr double SLIDER_MAX_POS = 10.0;
	inline static constexpr double DEFAULT_PARTICLE_MASS = 1.0;
	inline static constexpr double DEFAULT_PARTICLE_CHARGE = 0.1;
	inline static constexpr bool DEFAULT_PARTICLE_MOVABLE = true;
	inline static constexpr Types::Vec3d DEFAULT_PARTICLE_POS = Types::Vec3d(0.0);
	inline static const char* UNIT_PREFIXES[] = { "none", "n", "m", "k" };

	inline static constexpr int RK4_MASK = 1 << 0;
	inline static constexpr int BACKWARD_EULER_MASK = 1 << 1;

	// PRESETS
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