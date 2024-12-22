#include "Simulation.h"

void Simulation::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(Constants::WIDTH, Constants::HEIGHT, "Simulation", nullptr, nullptr);
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

void Simulation::displayParticleList()
{
    static ImVec2 previousSize = ImVec2(0.0f, 0.0f);
    ImGui::SetNextWindowPos(ImVec2(m_rendererHandle.getFramebufferWidth() / 2.0f - (previousSize.x / 2.0f), 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(300, 100));
    if (!ImGui::Begin("Particles"))
    {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }
	previousSize = ImGui::GetWindowSize();
    for (auto& particle : m_particles)
    {
        if (ImGui::CollapsingHeader(particleHeaderText(particle).c_str())) 
        {
            float mass = particle.getMass();
            ImGui::Text("Mass: ");
			ImGui::InputFloat(std::format("##mass{}", particle.getId()).c_str(), &mass);
			ImGui::Text("Affecting Force: ");
			ImGui::SliderScalar(std::format("##affectingForce{}x", particle.getId()).c_str(), ImGuiDataType_Double, &particle.affectingForceData().x, &SLIDER_MIN_AFFECTING_FORCE, &SLIDER_MAX_AFFECTING_FORCE);
            ImGui::SliderScalar(std::format("##affectingForce{}y", particle.getId()).c_str(), ImGuiDataType_Double, &particle.affectingForceData().y, &SLIDER_MIN_AFFECTING_FORCE, &SLIDER_MAX_AFFECTING_FORCE);
            ImGui::SliderScalar(std::format("##affectingForce{}z", particle.getId()).c_str(), ImGuiDataType_Double, &particle.affectingForceData().z, &SLIDER_MIN_AFFECTING_FORCE, &SLIDER_MAX_AFFECTING_FORCE);
			ImGui::Text("Position: ");
			ImGui::SliderScalar(std::format("##pos{}x", particle.getId()).c_str(), ImGuiDataType_Double, &particle.posData().x, &SLIDER_MIN_POS, &SLIDER_MAX_POS);
            ImGui::SliderScalar(std::format("##pos{}y", particle.getId()).c_str(), ImGuiDataType_Double, &particle.posData().y, &SLIDER_MIN_POS, &SLIDER_MAX_POS);
            ImGui::SliderScalar(std::format("##pos{}z", particle.getId()).c_str(), ImGuiDataType_Double, &particle.posData().z, &SLIDER_MIN_POS, &SLIDER_MAX_POS);
			particle.setMass(mass);
        }
    }
	ImGui::End();
    ImGui::PopStyleVar();
}

