#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Input.h"

class Camera {
public:
    Camera() : m_position(glm::vec3(0.0f, 0.0f, 0.0f)), m_pitch(0.0f), m_yaw(0.0f)
    {

    }

    Camera(glm::vec3 position, float pitch, float yaw, glm::vec3 up) 
        : m_position(position), m_pitch(pitch), m_yaw(yaw) 
    {

    }

    glm::mat4 getViewMatrix()
    {
        glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), m_position);
        glm::mat4 cameraRotation = getRotationMatrix();
        return glm::inverse(cameraTranslation * cameraRotation);
    }

    glm::mat4 getRotationMatrix()
    {
        glm::quat pitchRotation = glm::angleAxis(m_pitch, glm::vec3{ 1.f, 0.f, 0.f });
        glm::quat yawRotation = glm::angleAxis(m_yaw, glm::vec3{ 0.f, 1.f, 0.f });

        return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
    }


	glm::vec3& velocity() { return m_velocity; }
    glm::vec3& position() { return m_position; }
    void setVelocityX(float&& velocityX) { m_velocity.x = velocityX; }
    void setVelocityY(float&& velocityY) { m_velocity.y = velocityY; }
    void setVelocityZ(float&& velocityZ) { m_velocity.z = velocityZ; }
    void setPitch(float pitch) { m_pitch = pitch; }
    void setYaw(float yaw) { m_yaw = yaw; }
	bool locked() const { return m_locked; }
	void lock(bool locked) { m_locked = locked; }
	const float& getMaxVelocity() const { return m_maxVelocity; }

    void processKeyboardInput()
    {
        if (Input::isPressed(GLFW_KEY_W) && !Input::isPressed(GLFW_KEY_S)) 
            { m_velocity.z = m_maxVelocity; }
        else if (Input::isPressed(GLFW_KEY_S) && !Input::isPressed(GLFW_KEY_W)) 
            { m_velocity.z = -m_maxVelocity; }
        else { m_velocity.z = 0.0f; }

        if (Input::isPressed(GLFW_KEY_A) && !Input::isPressed(GLFW_KEY_D)) 
            { m_velocity.x = -m_maxVelocity; }
        else if (Input::isPressed(GLFW_KEY_D) && !Input::isPressed(GLFW_KEY_A)) 
            { m_velocity.x = m_maxVelocity; }
		else 
            { m_velocity.x = 0.0f; }

        if (Input::isJustPressed(GLFW_KEY_C)) { m_locked = !m_locked; }
    }

    void processMouseInput()
    {
        
		if (m_locked)
		{
			return;
		}
        
        float xoffset = Input::getMousePos().x - m_lastX;
        float yoffset = m_lastY - Input::getMousePos().y;
        m_lastX = Input::getMousePos().x;
        m_lastY = Input::getMousePos().y;

        xoffset *= m_sensitivity;
        yoffset *= m_sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

		m_pitch = std::clamp(m_pitch, glm::radians(-89.0f), glm::radians(89.0f));
        //std::cout << "x: " << xpos << "y: " << ypos << '\n';
		std::cout << "pitch: " << m_pitch << " yaw: " << m_yaw << std::endl;
    }

    void update()
    {
		processKeyboardInput();
		processMouseInput();
        glm::mat4 cameraRotation = getRotationMatrix();
        m_position += glm::vec3(cameraRotation * glm::vec4(m_velocity * 0.5f, 0.f));
    }

    void calculatePitchYaw() 
    {
        glm::vec3 direction = glm::normalize(m_target - m_position);
        m_yaw = atan2(direction.y, direction.x);
        m_pitch = asin(direction.z);
    }

private:
    bool m_locked = true;
    glm::vec3 m_velocity = glm::vec3(0.0f, 0.0f, 0.0f);;
    glm::vec3 m_position;
    glm::vec3 m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    float m_pitch{ 0.f };
    float m_yaw{ 0.f };
	const float m_maxVelocity{ 0.05f };
	double m_lastX{ 0.0 };
	double m_lastY{ 0.0 };
    float m_sensitivity = 0.01f;
};
