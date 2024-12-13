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
    void setPitch(float pitch) { m_pitch = pitch; update(); }
    void setYaw(float yaw) { m_yaw = yaw; update(); }
	bool locked() const { return m_locked; }
	void lock(bool locked) { m_locked = locked; }
	const float& getMaxVelocity() const { return m_maxVelocity; }

    void processMouseInput(double xpos, double ypos)
    {
		if (m_locked)
		{
			return;
		}
        float xoffset = xpos - m_lastX;
        float yoffset = m_lastY - ypos;
        m_lastX = xpos;
        m_lastY = ypos;

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
	float m_maxVelocity{ 0.05f };
	double m_lastX{ 0.0 };
	double m_lastY{ 0.0 };
    float m_sensitivity = 0.01f;
};
