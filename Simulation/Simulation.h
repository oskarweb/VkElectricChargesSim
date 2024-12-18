#pragma once

#include "VulkanRenderer.h"
#include "Camera.h"


class Simulation
{
public:
	Simulation() {}

	void run()
	{
		initWindow();
		glfwSetWindowUserPointer(m_window, this);
		m_renderer.setWindow(m_window);
		m_camera = Camera(glm::vec3(2.0f, 2.0f, 1.0f), 0.0f, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		m_renderer.setCamera(&m_camera);
		m_renderer.init();
		glfwSetKeyCallback(m_window, keyCallback);
		glfwSetCursorPosCallback(m_window, mouseCallback);
		glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
		glfwMakeContextCurrent(m_window);

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		ImGui::StyleColorsDark();

		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
			m_camera.update();
			m_renderer.drawFrame();
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			{
				static float f = 0.0f;
				static int counter = 0;

				m_renderer.m_renderableObjects[1].transformMatrix = glm::translate(m_renderer.m_renderableObjects[1].transformMatrix, glm::vec3(0.0, f, 0));
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
				ImGui::Text("This is some useful text.");  
				ImGui::SliderFloat("float", &f, -0.1f, 0.1f);
				ImGui::ColorEdit3("clear color", (float*)&clear_color);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}

			ImGui::Render();
			m_renderer.recordImguiData(ImGui::GetDrawData());
		}

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		m_renderer.cleanup();
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

private:
	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void initWindow();

	GLFWwindow* m_window = nullptr;
	VulkanRenderer m_renderer;
	Camera m_camera;
};