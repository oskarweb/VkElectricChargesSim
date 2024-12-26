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

void Simulation::removeParticle(std::vector<Particle>::iterator it) 
{
    if (it != m_particles.end())
    {
        (*it).cleanup();
        m_particles.erase(it);
    }
}

void Simulation::calculateParticlePositions()
{
	int steps = static_cast<int>(m_simulationTime / PARTICLE_TIME_STEP_S);
	
	for (auto& particle : m_particles)
	{
		particle.setInitialState();
	}

	for (int i = 0; i < steps; ++i)
	{
		for (auto& particle : m_particles)
		{
			Types::Vec3d force{ 0.0 };
			for (auto& particle_other : m_particles)
			{
				if (particle.getId() != particle_other.getId())
				{
					force += particle.getCoulombForce(particle_other);
				}
			}
			particle.setAffectingForce(force);
			particle.setAcceleration(force / particle.getMass());
			particle.setVelocity(particle.getVelocity() + (particle.getAcceleration() * PARTICLE_TIME_STEP_S));
			particle.setPos(particle.getPos() + (particle.getVelocity() * PARTICLE_TIME_STEP_S));
			particle.pushState();
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
		m_camera.update();
		m_rendererHandle.drawFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		ImGui::ShowDemoWindow();

		displayMainCtrlWindow();
		displayParticleListWindow();
		displayParticleAddWindow();

		if (not m_paused && m_simulateFromPrecalculatedSteps)
		{
			m_elapsedTime = std::clamp(m_elapsedTime + m_rendererHandle.getDeltaTime(), 0.0, m_simulationTime);
			if (m_elapsedTime == m_simulationTime)
			{
				m_paused = true;
			}
		}
		else if (not m_paused)
		{
			m_elapsedTime += m_rendererHandle.getDeltaTime();
		}

		if (not m_paused && m_simulateFromPrecalculatedSteps)
		{
			for (auto& particle : m_particles)
			{
				if (particle.isMovable())
				{
					particle.updateFromPrecalcPos(static_cast<uint32_t>(m_elapsedTime / PARTICLE_TIME_STEP_S));
				}
			}
		}
		else if (not m_paused)
		{
			for (auto& particle : m_particles)
			{
				if (particle.isMovable())
				{
					particle.update(m_elapsedTime);
				}
			}
		}

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
	ImGui::Text("Time Elapsed: %fs", m_elapsedTime);
	ImGui::Text("Position: %f, %f", mousePositionRelative.x, mousePositionRelative.y);
	ImGui::Text("Own Delta Time: %f", m_rendererHandle.getDeltaTime());
	ImGui::Text("ImGui Delta Time: %f", ImGui::GetIO().DeltaTime);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::InputDouble("Simulation Time (s)", &m_simulationTime);

	ImGui::Checkbox("Simulate from precalculated steps", &m_simulateFromPrecalculatedSteps);
	
	if (ImGui::Button("Start") && m_paused)
	{
		if (m_elapsedTime == m_simulationTime)
		{
			restartSimulation();
			m_elapsedTime = 0.0;
		}
		if (m_simulateFromPrecalculatedSteps)
		{
			for (auto& particle : m_particles)
			{
				particle.clearStates();
			}
			calculateParticlePositions();
		}
		m_paused = false;
	}

	if (ImGui::Button("Pause") && not m_paused)
	{
		m_paused = true;
	}

	if (ImGui::Button("Restart Simulation"))
	{
		restartSimulation();
	}

	if (ImGui::Button("Reset All"))
	{
		resetAll();
	}

	ImGui::End();
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
	for (auto& particle : m_particles)
	{
		particle.cleanup();
	}
	m_elapsedTime = 0.0;
	m_particles.clear();
	m_paused = true;
	m_simulateFromPrecalculatedSteps = true;
	Particle::resetId();
}

void Simulation::restartSimulation()
{
	m_paused = true;
	m_elapsedTime = 0.0;
	for (auto& particle : m_particles)
	{
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