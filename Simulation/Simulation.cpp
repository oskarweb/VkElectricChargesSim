#include "Simulation.h"

void Simulation::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    m_window = glfwCreateWindow(constants::WIDTH, constants::HEIGHT, "Simulation", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
}

void Simulation::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	Simulation* simulation = static_cast<Simulation*>(glfwGetWindowUserPointer(window));
	if (simulation)
	{
		simulation->m_camera.processMouseInput(xpos, ypos);
	}
}

void Simulation::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Simulation* simulation = static_cast<Simulation*>(glfwGetWindowUserPointer(window));

    switch (key)
    {
    case GLFW_KEY_W:
    {
        int sKeyState = glfwGetKey(window, GLFW_KEY_S);
        if (action == GLFW_PRESS)
        {
            simulation->m_camera.velocity() = glm::vec3(
                simulation->m_camera.velocity().x,
                simulation->m_camera.velocity().y,
                (constants::KEY_DIRECTION.at(key).z + ((sKeyState == GLFW_PRESS) ? constants::KEY_DIRECTION.at(GLFW_KEY_S).z : 0.f)) * simulation->m_camera.getMaxVelocity()
            );
        }
        else if (action == GLFW_RELEASE)
        {
            simulation->m_camera.velocity() = glm::vec3(
                simulation->m_camera.velocity().x,
                simulation->m_camera.velocity().y,
                (sKeyState == GLFW_REPEAT) ? constants::KEY_DIRECTION.at(GLFW_KEY_S).z * simulation->m_camera.getMaxVelocity() : 0.f
            );
        }
        break;
    }
    case GLFW_KEY_S:
    {
        int wKeyState = glfwGetKey(window, GLFW_KEY_W);
        if (action == GLFW_REPEAT)
        {
            simulation->m_camera.velocity() = glm::vec3(
                simulation->m_camera.velocity().x,
                simulation->m_camera.velocity().y,
                (constants::KEY_DIRECTION.at(key).z + ((wKeyState == GLFW_REPEAT) ? constants::KEY_DIRECTION.at(GLFW_KEY_W).z : 0.f)) * simulation->m_camera.getMaxVelocity()
            );
        }
        else if (action == GLFW_RELEASE)
        {
            simulation->m_camera.velocity() = glm::vec3(
                simulation->m_camera.velocity().x,
                simulation->m_camera.velocity().y,
                (wKeyState == GLFW_REPEAT) ? constants::KEY_DIRECTION.at(GLFW_KEY_W).z * simulation->m_camera.getMaxVelocity() : 0.f
            );
        }
        break;
    }
    case GLFW_KEY_A:
    {
        int dKeyState = glfwGetKey(window, GLFW_KEY_D);
        if (action == GLFW_REPEAT)
        {
            simulation->m_camera.velocity() = glm::vec3(
                (constants::KEY_DIRECTION.at(key).x + ((dKeyState == GLFW_REPEAT) ? constants::KEY_DIRECTION.at(GLFW_KEY_D).x : 0.f)) * simulation->m_camera.getMaxVelocity(),
                simulation->m_camera.velocity().y,
                simulation->m_camera.velocity().z
            );
        }
        else if (action == GLFW_RELEASE)
        {
            simulation->m_camera.velocity() = glm::vec3(
                (dKeyState == GLFW_REPEAT) ? constants::KEY_DIRECTION.at(GLFW_KEY_W).x * simulation->m_camera.getMaxVelocity() : 0.f,
                simulation->m_camera.velocity().y,
                simulation->m_camera.velocity().z
            );
        }
        break;
    }
    case GLFW_KEY_D:
    {
        int aKeyState = glfwGetKey(window, GLFW_KEY_A);
        if (action == GLFW_REPEAT)
        {
            simulation->m_camera.velocity() = glm::vec3(
                (constants::KEY_DIRECTION.at(key).x + ((aKeyState == GLFW_REPEAT) ? constants::KEY_DIRECTION.at(GLFW_KEY_A).x : 0.f)) * simulation->m_camera.getMaxVelocity(),
                simulation->m_camera.velocity().y,
                simulation->m_camera.velocity().z
            );
        }
        else if (action == GLFW_RELEASE)
        {
            simulation->m_camera.velocity() = glm::vec3(
                (aKeyState == GLFW_REPEAT) ? constants::KEY_DIRECTION.at(GLFW_KEY_W).x * simulation->m_camera.getMaxVelocity() : 0.f,
                simulation->m_camera.velocity().y,
                simulation->m_camera.velocity().z
            );
        }
        break;
    }
    case GLFW_KEY_F:
        if (action == GLFW_PRESS)
        {
            std::cout << "POS X: " << simulation->m_camera.position().x << "Y: " << simulation->m_camera.position().y << "Z: " << simulation->m_camera.position().z << '\n';
            simulation->m_camera.lock(!simulation->m_camera.locked());
            if (simulation->m_camera.locked())
            {
                //simulation->m_camera.position() = glm::vec3(2.0f, 2.0f, 3.0f);
                //simulation->m_camera.setPitch(1.0f);
                //simulation->m_camera.setYaw(-8.5f);
            }
        }
    }
    //std::cout << "X: " << simulation->m_camera.velocity().x << "Z: " << simulation->m_camera.velocity().z << '\n';
}