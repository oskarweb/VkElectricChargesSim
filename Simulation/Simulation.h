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

		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
			m_renderer.drawFrame();
			m_camera.update();
		}

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