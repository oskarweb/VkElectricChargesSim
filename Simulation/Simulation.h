#pragma once

#include "VulkanRenderer.h"
#include "Camera.h"
#include "Input.h"
#include "Models.h"
#include "Particle.h"

class Simulation
{
public:
	Simulation(VulkanRenderer& rendererHandle) : m_rendererHandle(rendererHandle), m_camera(Camera(glm::vec3(2.0f, 2.0f, 1.0f), 0.0f, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f)))
	{
	}

	void run()
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
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_CursorPosCallback(m_window, Input::mousePos.x, Input::mousePos.y);

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
			m_camera.update();
			m_rendererHandle.drawFrame();
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			{
				//static float x, y, z = 0.0f;
				static int counter = 0;

				for (auto& renderable : m_rendererHandle.m_renderableObjects)
				{
					//if (renderable.second.id == 1)
						//renderable.second.transformMatrix = glm::translate(renderable.second.transformMatrix, glm::vec3(0.0, f, 0));
				}
				//m_renderer.m_renderableObjects.equal_range["pyramidcube"][0].transformMatrix = glm::translate(m_renderer.m_renderableObjects[1].transformMatrix, glm::vec3(0.0, f, 0));
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::Begin("Options");
				bool isHovered = ImGui::IsItemHovered();
				bool isFocused = ImGui::IsItemFocused();
				ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
				ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
				ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);
				ImGui::Text("Is mouse over screen? %s", isHovered ? "Yes" : "No");
				ImGui::Text("Is screen focused? %s", isFocused ? "Yes" : "No");
				ImGui::Text("Position: %f, %f", mousePositionRelative.x, mousePositionRelative.y);
				ImGui::Text("Mouse clicked: %s", ImGui::IsMouseDown(ImGuiMouseButton_Left) ? "Yes" : "No");
				if (ImGui::Button("Add"))
				{
					addParticle(Particle(
						1.0f,
						1.0f,
						false,
						glm::vec3(0.0f, 0.0f, 1.0f),
						glm::vec3(0.0f, -10.0f, 0.0f)
					));
					std::cout << m_particles.size() << '\n';
				}
				if (ImGui::Button("Erase"))
				{
					removeParticle(m_particles.begin());
					std::cout << m_particles.size() << '\n';
				}

				//ImGui::SliderFloat("x", &forceArrow.force.x, -1.0f, 1.0f);
				//ImGui::SliderFloat("y", &forceArrow.force.y, -1.0f, 1.0f);
				//ImGui::SliderFloat("z", &forceArrow.force.z, -1.0f, 1.0f);
				ImGui::Text("This is some useful text.");  
				ImGui::ColorEdit3("clear color", (float*)&clear_color);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}
			/*
			for (auto& [name, renderableIt] : forceArrow.renderables)
			{
				if (renderableIt != m_renderer.m_renderableObjects.end())
					m_renderer.removeRenderable(renderableIt);
			}
			forceArrow.renderables.clear();
			*/
			ImGui::Render();
			m_rendererHandle.recordImguiData(ImGui::GetDrawData());
			//Input::process();
		}

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_rendererHandle.cleanup();
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

private:
	void addParticle(Particle&& particle);
	void removeParticle(std::vector<Particle>::iterator it);
	void initWindow();

	std::vector<Particle> m_particles;

	GLFWwindow* m_window = nullptr;
	VulkanRenderer& m_rendererHandle;
	Camera m_camera;
};