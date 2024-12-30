#include "Simulation.h"

void Simulation::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(Constants::WIDTH, Constants::HEIGHT, "Simulation", nullptr, nullptr);
    glfwSetWindowSizeLimits(m_window, Constants::WIDTH, Constants::HEIGHT, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetWindowUserPointer(m_window, this);
}

void Simulation::addParticle(Particle&& particle)
{
	
    auto& particleRef = m_particles.emplace_back(std::move(particle));
}

void Simulation::removeParticle(std::vector<Particle>::iterator& it) 
{
    if (it != m_particles.end())
    {
        (*it).cleanup();
        m_particles.erase(it);
    }
}

void Simulation::launchParticleThreads()
{
	for (uint32_t i = 0; i < m_threads.size(); ++i)
	{
		uint32_t minIdx = i * static_cast<uint32_t>(m_particles.size()) / NUM_THREADS;
		uint32_t maxIdx = (i + 1) * static_cast<uint32_t>(m_particles.size()) / NUM_THREADS;
        m_threads[i] = std::jthread(&Simulation::calculateParticlePostionsThreaded, this, std::stop_token{}, minIdx, maxIdx);
	}
}

void Simulation::calculateParticlePostionsThreaded(std::stop_token stopToken, uint32_t minIdx, uint32_t maxIdx)
{
	while (m_elapsedTime <= m_simulationTime && not m_paused)
	{
		if (stopToken.stop_requested())
		{
			std::cout << "thread stop requested" << '\n';
		}
		for (uint32_t i = minIdx; i < maxIdx; ++i)
		{
			calculatePositionsForSingleParticle(&m_particles[i]);
		}
	}
}

void Simulation::calculatePositionsForSingleParticle(Particle* particle)
{
	uint32_t stepIdx = particle->getMaxStep() + 1;

	if (m_mutualMaxStep < (stepIdx - 1))
	{
		return;
	}

	if (particle->getBufferedStepCount() >= STEPS_BUFFERED_AT_ONCE)
	{
		return;
	}

	std::unique_lock<std::mutex> lk(particle->mutexData());

	Types::Vec3d force{ 0.0 };
	for (auto& particleOther : m_particles)
	{
		if (particle->getId() != particleOther.getId())
		{
			force += particle->getCoulombForce(stepIdx - 1, particleOther);
		}
	}
	auto acc = force / particle->getMass();
	auto vel = particle->statesData()[stepIdx - 1].velocity + (particle->statesData()[stepIdx - 1].acceleration * m_timeStep);
	auto pos = particle->statesData()[stepIdx - 1].pos + (particle->statesData()[stepIdx - 1].velocity * m_timeStep);
	particle->pushState(stepIdx, force, acc, vel, pos);

}

void Simulation::calculateParticlePositions()
{
	uint32_t startingStep = m_mutualMaxStep + 1;
	m_mutualMaxStep = std::min(m_mutualMaxStep + getStepsPer20ms(), static_cast<uint32_t>(m_simulationTime / m_timeStep));
	for (uint32_t i = startingStep; i < m_mutualMaxStep + 1; ++i)
	{
		for (auto& particle : m_particles)
		{
			Types::Vec3d force{ 0.0 };
			for (auto& particle_other : m_particles)
			{
				if (particle.getId() != particle_other.getId())
				{
					force += particle.getCoulombForce(i - 1, particle_other);
				}
			}
			auto acc = force / particle.getMass();
			auto vel = particle.statesData()[i - 1].velocity + acc * m_timeStep;
			auto pos = particle.statesData()[i - 1].pos + vel * m_timeStep;
			particle.pushState(i, force, acc, vel, pos);
		}
	}
}

void Simulation::calculateAllParticlePositions()
{
	uint32_t steps = static_cast<uint32_t>(m_simulationTime / m_timeStep) + 1;

	for (uint32_t i = 1; i < steps; ++i)
	{
		for (auto& particle : m_particles)
		{
			Types::Vec3d force{ 0.0 };
			for (auto& particle_other : m_particles)
			{
				if (particle.getId() != particle_other.getId())
				{
					force += particle.getCoulombForce(i - 1, particle_other);
				}
			}
			particle.setAffectingForce(force);
			particle.setAcceleration(force / particle.getMass());
			particle.setVelocity(particle.getVelocity() + (particle.getAcceleration() * m_timeStep));
			particle.setPos(particle.getPos() + (particle.getVelocity() * m_timeStep));
			particle.pushState(i);
		}
	}
}

void Simulation::run()
{
	initWindow();
	Input::setWindow(m_window);
	glfwSetWindowUserPointer(m_window, this);
	m_rendererHandle.setWindow(m_window);
	m_rendererHandle.setCamera(&m_camera);
	m_rendererHandle.init();
	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwMakeContextCurrent(m_window);

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	ImFontConfig config;
	config.OversampleH = 1;
	config.OversampleV = 1;
	std::filesystem::path arimoPath = Constants::FONTS_PATH / "arimo" / "Arimo-Regular.ttf";
	io.Fonts->AddFontFromFileTTF(arimoPath.string().c_str(), 16.0f, &config, io.Fonts->GetGlyphRangesGreek());
	io.Fonts->Build();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_CursorPosCallback(m_window, Input::mousePos.x, Input::mousePos.y);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	AxesModel axes(glm::vec3(0.0f));
	m_rendererHandle.addRenderables(&axes);

	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();
		m_camera.update(static_cast<float>(m_rendererHandle.getDeltaTime()));
		m_rendererHandle.drawFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		if (m_threadedCalculation)
		{
			m_mutualMaxStep = UINT32_MAX;
			for (auto& particle : m_particles)
			{
				m_mutualMaxStep = std::min(m_mutualMaxStep.load(), particle.getMaxStep());
			}
			m_mutualStepCv.notify_all();
		}

		if (not m_precalculateAll && not m_threadedCalculation && m_simulateFromPrecalculatedSteps) // Precalculated 100ms ahead
		{
			static uint32_t skipCounter = 0;
			static double lastCalcTime = 0;
			m_maxUsedStep = m_elapsedTime / m_timeStep;

			if (not m_paused && skipCounter == 0)
			{
				if (m_maxUsedStep <= m_mutualMaxStep)
				{
					updatePostions();
					m_elapsedTime = std::clamp(m_elapsedTime + std::min(m_rendererHandle.getDeltaTime(), 0.02), 0.0, m_simulationTime);
					for (auto& particle : m_particles)
					{
						std::erase_if(particle.statesData(), [this](const auto& item)
						{
								auto const& [key, value] = item;
								return key < m_maxUsedStep;
						});
					}
				}
				auto start = std::chrono::high_resolution_clock::now();
				calculateParticlePositions();
				lastCalcTime = std::chrono::duration<double, std::chrono::seconds::period>(start - std::chrono::high_resolution_clock::now()).count();
				//skipCounter = 2;
			}
			else if (not m_paused && skipCounter > 0)
			{ 
				skipCounter--;
			}
		}

		if (m_precalculateAll && not m_threadedCalculation && m_simulateFromPrecalculatedSteps && not m_paused)
		{
			if (m_skipUpdate)
			{
				m_skipUpdate = false;
			}
			else
			{
				m_maxUsedStep = m_elapsedTime / m_timeStep;
				updatePostions();
				m_elapsedTime = std::clamp(m_elapsedTime + m_rendererHandle.getDeltaTime(), 0.0, m_simulationTime);
				if (m_elapsedTime == m_simulationTime)
				{
					m_paused = true;
				}
			}
		}
		
		if (not m_paused && not m_simulateFromPrecalculatedSteps) // Real time update loop
		{
			if (m_skipUpdate)
			{
				m_skipUpdate = false;
			}
			else
			{
				for (auto& particle : m_particles)
				{
					if (particle.isMovable())
					{
						particle.update(m_elapsedTime);
					}
				}
			}
		}
		
		if (not m_paused && not m_simulateFromPrecalculatedSteps)
		{
			if (m_skipUpdate)
			{
				m_skipUpdate = false;
			}
			else
			{
				m_elapsedTime += m_rendererHandle.getDeltaTime();
			}
		}

		if (m_elapsedTime == m_simulationTime)
		{
			m_paused = true;
		}

		displayMainCtrlWindow();
		displayParticleListWindow();
		displayParticleAddWindow();

		ImGui::Render();
		m_rendererHandle.recordImguiData(ImGui::GetDrawData());
	}

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	m_rendererHandle.cleanup();
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void Simulation::updatePositionsThreaded()
{
	if (not m_paused && m_simulateFromPrecalculatedSteps && m_isHung) // Handle case when there was a lack of particle step data in previous iteration
	{
		m_isHung = false;
		for (std::vector<Particle>::iterator it = m_hungIt; it != m_particles.end(); it++)
		{
			std::unique_lock<std::mutex> lk((*it).mutexData());
			if (not (*it).isMovable())
			{
				continue;
			}
			bool updateSuccess = (*it).updateFromPrecalcPos(static_cast<uint32_t>(m_elapsedTime / m_timeStep));
			if (not updateSuccess && (m_elapsedTime <= m_simulationTime))
			{
				m_hungIt = it;
				m_isHung = true;
				break;
			}
		}
	}
	else if (not m_paused && m_simulateFromPrecalculatedSteps && not m_isHung)
	{
		for (auto it = m_particles.begin(); it != m_particles.end(); it++) // Loop for precalculated simulation
		{
			std::unique_lock<std::mutex> lk((*it).mutexData());
			if (not (*it).isMovable())
			{
				continue;
			}
			bool updateSuccess = (*it).updateFromPrecalcPos(static_cast<uint32_t>(m_elapsedTime / m_timeStep));
			if (not updateSuccess && (m_elapsedTime <= m_simulationTime))
			{
				m_hungIt = it;
				m_isHung = true;
				break;
			}
		}
	}
}

bool Simulation::updatePostions()
{
	for (auto it = m_particles.begin(); it != m_particles.end(); it++)
	{
		if (not (*it).isMovable())
		{
			continue;
		}
		bool updateSuccess = (*it).updateFromPrecalcPos(static_cast<uint32_t>(m_maxUsedStep));
	}
	return true;
}

void Simulation::displayMainCtrlWindow()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowBgAlpha(WINDOWS_BG_ALPHA);
	ImGui::SetNextWindowSizeConstraints(MAIN_CTRL_WINDOW_MIN_SIZE, ImVec2(m_particleAddWindowInfo.pos.x, static_cast<float>(m_rendererHandle.getFramebufferHeight()) / 2.0f));
	if (!ImGui::Begin("Options"))
	{
		ImGui::End();
		return;
	}
	m_mainCtrlWindowInfo.size = ImGui::GetWindowSize();
	m_mainCtrlWindowInfo.pos = ImGui::GetWindowPos();

	ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
	ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
	ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);
	ImGui::Text("Time Elapsed: %fs", m_elapsedTime.load());
	ImGui::Text("Position: %f, %f", mousePositionRelative.x, mousePositionRelative.y);
	ImGui::Text("Own Delta Time: %f", m_rendererHandle.getDeltaTime());
	ImGui::Text("ImGui Delta Time: %f", ImGui::GetIO().DeltaTime);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	static double simulationTime = DEFAULT_SIMULATION_TIME;
	static double timeStep = DEFAULT_TIME_STEP;

	ImGui::InputDouble("Sim Time (s)", &simulationTime);

	ImGui::InputDouble("Time step(s)", &timeStep);

	ImGui::Checkbox("Simulate from precalculated steps", &m_simulateFromPrecalculatedSteps);
	
	ImGui::Checkbox("Precalculate all steps", &m_precalculateAll);

	ImGui::Checkbox("Multithreading(EXPERIMENTAL)", &m_threadedCalculation);

	if (ImGui::Button("Start") && m_paused)
	{
		m_timeStep = timeStep;
		if (getStepsPer20ms() <= getMaxStepsBuffered())
		{
			m_simulationTime = simulationTime;
			m_skipUpdate = true;
			startSimulation();
		}
	}

	if (getStepsPer20ms() > getMaxStepsBuffered())
	{
		ImGui::SameLine();
		ImGui::Text("Reduce Time Step or amount of particles");
	}

	if (ImGui::Button("Pause") && not m_paused)
	{
		m_paused = true;
	}

	ImGui::SameLine();
	ImGui::Text("Paused: %s", m_paused ? "true" : "false");
	ImGui::SameLine();
	ImGui::Text("Hung: %s", m_isHung ? "true" : "false");

	if (ImGui::Button("Resume") && m_paused)
	{
		m_paused = false;
	}

	if (ImGui::Button("Restart Simulation") && m_paused) 
	{
		restartSimulation();
	}

	if (ImGui::Button("Reset All") && m_paused)
	{
		resetAll();
	}

	ImGui::Text("Steps per 100ms: %d", getStepsPer20ms());

	ImGui::Text("Max buffered: %d", getMaxStepsBuffered());

	ImGui::End();
}

void Simulation::startSimulation()
{
	if (m_elapsedTime == m_simulationTime)
	{
		restartSimulation();
		m_elapsedTime = 0.0;
	}
	if (m_simulateFromPrecalculatedSteps && m_particles.size() > 0 && not m_precalculateAll)
	{
		for (auto& particle : m_particles)
		{
			if (particle.initialStateData())
			{
				particle.setAffectingForce(particle.getInitialState().affectingForce);
				particle.setAcceleration(particle.getInitialState().acceleration);
				particle.setVelocity(particle.getInitialState().velocity);
				particle.setPos(particle.getInitialState().pos);
				particle.update();
			}
			particle.clearStates();
			particle.pushState(0);
		}
		if (m_threadedCalculation)
		{
			launchParticleThreads();
		}
	}
	else if (m_simulateFromPrecalculatedSteps && m_particles.size() > 0 && m_precalculateAll)
	{
		for (auto& particle : m_particles)
		{
			if (particle.initialStateData())
			{
				particle.setAffectingForce(particle.getInitialState().affectingForce);
				particle.setAcceleration(particle.getInitialState().acceleration);
				particle.setVelocity(particle.getInitialState().velocity);
				particle.setPos(particle.getInitialState().pos);
				particle.update();
			}
			particle.clearStates();
			particle.pushState(0);
		}
		calculateAllParticlePositions();
	}
	m_paused = false;
}

void Simulation::displayParticleListWindow()
{
    std::vector<std::vector<Particle>::iterator> particlesToRemove;
    ImGui::SetNextWindowPos(ImVec2(m_rendererHandle.getFramebufferWidth() - m_particleListWindowInfo.size.x, 0.0f));
	ImGui::SetNextWindowBgAlpha(WINDOWS_BG_ALPHA);
	ImGui::SetNextWindowSizeConstraints(PARTICLE_LIST_WINDOW_MIN_SIZE, ImVec2(m_rendererHandle.getFramebufferWidth() - MAIN_CTRL_WINDOW_MIN_SIZE.x - PARTICLE_ADD_WINDOW_MIN_SIZE.x, m_rendererHandle.getFramebufferHeight() * 0.9f));
    if (!ImGui::Begin("Particles"))
    {
        ImGui::End();
        return;
    }
	m_particleListWindowInfo.size = ImGui::GetWindowSize();
	m_particleListWindowInfo.pos = ImGui::GetWindowPos();
    if (ImGui::BeginTable("ParticleTable", 5))
    {
        ImGui::TableSetupColumn("Particle ID");
        ImGui::TableSetupColumn("Pos");
        ImGui::TableSetupColumn("Vel");
		ImGui::TableSetupColumn("Acc");
		ImGui::TableSetupColumn("F");
		ImGui::TableHeadersRow();
        for (auto it = m_particles.begin(); it != m_particles.end(); ++it)
        {
            Particle& particle = *it;
            ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Button(std::format("X##{}", particle.getId()).c_str()) && m_paused)
			{
				particlesToRemove.push_back(it);
			}
            ImGui::SameLine();
            if (ImGui::CollapsingHeader(particleHeaderText(particle).c_str()) && 
				((m_simulateFromPrecalculatedSteps && (m_elapsedTime == 0.0 || m_elapsedTime == m_simulationTime)) || not m_simulateFromPrecalculatedSteps)
			)
            {
				ImGui::Text("Charge: ");
				ImGui::SameLine();
				ImGui::InputDouble(std::format("##massl{}", particle.getId()).c_str(), &particle.chargeData());
				ImGui::Text("Mass: ");
				ImGui::SameLine();
				ImGui::InputDouble(std::format("##chargel{}", particle.getId()).c_str(), &particle.massData());
				ImGui::Text("X: ");
				ImGui::SameLine();
				ImGui::InputDouble(std::format("##posxl{}", particle.getId()).c_str(), &particle.posData().x);
				ImGui::Text("Y: ");
				ImGui::SameLine();
				ImGui::InputDouble(std::format("##posyl{}", particle.getId()).c_str(), &particle.posData().y);
				ImGui::Text("Z: ");
				ImGui::SameLine();
				ImGui::InputDouble(std::format("##poszl{}", particle.getId()).c_str(), &particle.posData().z);
            }
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(Helpers::vectorFormat(particle.getPos()).c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::Text(Helpers::vectorFormat(particle.getVelocity()).c_str());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text(Helpers::vectorFormat(particle.getAcceleration()).c_str());
			ImGui::TableSetColumnIndex(4);
			ImGui::Text(Helpers::vectorFormat(particle.getAffectingForce()).c_str());
        }
		ImGui::EndTable();
    }
	ImGui::End();
    for (auto& it : particlesToRemove)
    {
        removeParticle(it);
    }
}

void Simulation::displayParticleAddWindow()
{
    ImGui::SetNextWindowPos(ImVec2(m_rendererHandle.getFramebufferWidth() - m_particleListWindowInfo.size.x - m_particleAddWindowInfo.size.x, 0.0f));
	ImGui::SetNextWindowBgAlpha(WINDOWS_BG_ALPHA);
    ImGui::SetNextWindowSizeConstraints(PARTICLE_ADD_WINDOW_MIN_SIZE, ImVec2(m_rendererHandle.getFramebufferWidth() - m_mainCtrlWindowInfo.size.x - m_particleListWindowInfo.size.x, static_cast<float>(m_rendererHandle.getFramebufferHeight()) / 2.0f));
    if (!ImGui::Begin("Add Particle"))
    {
        ImGui::End();
        return;
    }
	m_particleAddWindowInfo.size = ImGui::GetWindowSize();
	m_particleAddWindowInfo.pos = ImGui::GetWindowPos();

	static double charge = DEFAULT_PARTICLE_CHARGE;
	static double mass = DEFAULT_PARTICLE_MASS;
	static bool movable = DEFAULT_PARTICLE_MOVABLE;
	static Types::Vec3d pos = DEFAULT_PARTICLE_POS;

	static int chargePrefixIdx = 2;
	static int massPrefixIdx = 3;
	static int distancePrefixIdx = 0;

	const std::string& chargeText = std::format("Charge [{}C]", UNIT_PREFIXES[chargePrefixIdx] == "none" ? "" : UNIT_PREFIXES[chargePrefixIdx]);
	ImGui::Text(chargeText.c_str());
	ImGui::SameLine();
	displayUnitSelector(chargeText, chargePrefixIdx);
	ImGui::InputDouble("##charge", &charge);
	
	const std::string& massText = std::format("Mass [{}g]", UNIT_PREFIXES[massPrefixIdx] == "none" ? "" : UNIT_PREFIXES[massPrefixIdx]);
	ImGui::Text(massText.c_str());
	ImGui::SameLine();
	displayUnitSelector(massText, massPrefixIdx);
	ImGui::InputDouble("##mass", &mass);

	const std::string& distanceText = std::format("Pos [{}m]", UNIT_PREFIXES[distancePrefixIdx] == "none" ? "" : UNIT_PREFIXES[distancePrefixIdx]);
	ImGui::Text(distanceText.c_str());
	ImGui::SameLine();
	displayUnitSelector(distanceText, distancePrefixIdx);
	ImGui::Text("X: ");
	ImGui::SameLine();
	ImGui::InputDouble("##posx", &pos.x);
	ImGui::Text("Y: ");
	ImGui::SameLine();
	ImGui::InputDouble("##posy", &pos.y);
	ImGui::Text("Z: ");
	ImGui::SameLine();
	ImGui::InputDouble("##posz", &pos.z);
	
	ImGui::Text("Movable: ");
	ImGui::Checkbox("##movable", &movable);
	
	if (ImGui::Button("Add") && ((m_paused && m_simulateFromPrecalculatedSteps) || (not m_simulateFromPrecalculatedSteps)))
	{
		addParticle(Particle(
			charge * Constants::unitPrefixFactor<double>(UNIT_PREFIXES[chargePrefixIdx] == "none" ? ' ' : UNIT_PREFIXES[chargePrefixIdx][0]),
			mass * Constants::unitPrefixFactor<double>(UNIT_PREFIXES[massPrefixIdx] == "none" ? ' ' : UNIT_PREFIXES[massPrefixIdx][0]) / 1000.0,
			movable, 
			Types::Vec3d(pos.x , pos.y, pos.z) * Constants::unitPrefixFactor<double>(UNIT_PREFIXES[distancePrefixIdx] == "none" ? ' ' : UNIT_PREFIXES[distancePrefixIdx][0])
		));
		charge = DEFAULT_PARTICLE_CHARGE;
		mass = DEFAULT_PARTICLE_MASS;
		movable = DEFAULT_PARTICLE_MOVABLE;
		pos = DEFAULT_PARTICLE_POS;
	}

	if (ImGui::Button("Preset1") && ((m_paused && m_simulateFromPrecalculatedSteps) || (not m_simulateFromPrecalculatedSteps)))
	{
		for (const auto& particleConfig : PARTICLE_PRESET1)
		{
			addParticle(Particle(
				particleConfig.charge,
				particleConfig.mass,
				particleConfig.movable,
				particleConfig.pos
			));
		}
	}

	if (ImGui::Button("Preset2") && ((m_paused && m_simulateFromPrecalculatedSteps) || (not m_simulateFromPrecalculatedSteps)))
	{
		for (const auto& particleConfig : PARTICLE_PRESET2)
		{
			addParticle(Particle(
				particleConfig.charge,
				particleConfig.mass,
				particleConfig.movable,
				particleConfig.pos
			));
		}
	}

	if (ImGui::Button("Preset3") && ((m_paused && m_simulateFromPrecalculatedSteps) || (not m_simulateFromPrecalculatedSteps)))
	{
		for (const auto& particleConfig : PARTICLE_PRESET3)
		{
			addParticle(Particle(
				particleConfig.charge,
				particleConfig.mass,
				particleConfig.movable,
				particleConfig.pos
			));
		}
	}

    ImGui::End();
}

void Simulation::resetAll()
{
	m_skipUpdate = true;
	m_paused = true;
	m_mutualMaxStep = 0;
	m_elapsedTime = 0.0;
	if (m_threadedCalculation)
	{
		for (int i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i].joinable()) {
				m_threads[i].join();
			}
		}
	}

	for (auto& particle : m_particles)
	{
		particle.cleanup();
	}
	m_particles.clear();
	m_simulateFromPrecalculatedSteps = true;
	Particle::resetId();
}

void Simulation::restartSimulation()
{
	m_skipUpdate = true;
	m_paused = true;
	m_mutualMaxStep = 0;
	m_elapsedTime = 0.0;

	if (m_threadedCalculation)
	{
		for (int i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i].joinable()) {
				m_threads[i].join();
			}
		}
	}

	for (auto& particle : m_particles)
	{
		particle.resetTrail();
		particle.statesData().clear();
		particle.setMaxStep(0);
		particle.setAffectingForce(particle.getInitialState().affectingForce);
		particle.setAcceleration(particle.getInitialState().acceleration);
		particle.setVelocity(particle.getInitialState().velocity);
		particle.setPos(particle.getInitialState().pos);
		particle.update();
	}

}

void Simulation::displayUnitSelector(const std::string& unit, int& prefixIdx)
{
	if (ImGui::BeginCombo(("##"+unit).c_str(), UNIT_PREFIXES[prefixIdx]))
	{
		for (int n = 0; n < IM_ARRAYSIZE(UNIT_PREFIXES); n++)
		{
			const bool is_selected = (prefixIdx == n);
			const char* selectableText = UNIT_PREFIXES[n] == "none" ? "" : UNIT_PREFIXES[n];
			if (ImGui::Selectable(UNIT_PREFIXES[n], is_selected))
			{
				prefixIdx = n;
			}
		}
		ImGui::EndCombo();
	}
}