#pragma once 

#include "Mesh.h"

struct Material
{
	std::string name = "default";
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	Material() : pipeline(VK_NULL_HANDLE), pipelineLayout(VK_NULL_HANDLE) {}
	Material(const std::string& name, VkPipeline pipeline, VkPipelineLayout pipelineLayout) : name(name), pipeline(pipeline), pipelineLayout(pipelineLayout) {}
};

struct Renderable
{
	Mesh* mesh;
	Material* material;
    glm::mat4 transformMatrix;
	std::unique_ptr<int> textureIdx = nullptr;
	uint64_t id = 0;

};

struct RenderableComp
{
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
        return lhs < rhs;
    }
};

struct Image
{
	VkImage image;
	VkDeviceMemory imageMemory;
};

struct Texture
{
	Image image;
	VkImageView imageView;
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct CameraBuffer {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct RenderableInfo
{
    std::string renderableName;
    std::string meshName;
    std::string materialName;
    glm::mat4 transformMatrix;
};

inline const std::vector<Vertex> cubeVertices = {
    // Front
    {{ 0.0, 1.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},

    {{ 0.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 0.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    // Back
    {{ 1.0, 1.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.0, 1.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.0, 0.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

    {{ 0.0, 0.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 0.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    // Left
    {{ 0.0, 1.0, 1.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0, 1.0, 0.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0, 0.0, 0.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},

    {{ 0.0, 0.0, 0.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0, 0.0, 1.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0, 1.0, 1.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    // Right
    {{ 1.0, 1.0, 0.0}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 1.0}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 0.0, 1.0}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},

    {{ 1.0, 0.0, 1.0}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 0.0, 0.0}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 0.0}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    // Top
    {{ 0.0, 0.0, 0.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 0.0, 0.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.0, 0.0, 1.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},

    {{ 0.0, 0.0, 1.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 0.0, 0.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 0.0, 1.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    // Bottom
    {{ 0.0, 1.0, 1.0}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 0.0}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.0, 1.0, 0.0}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},

    {{ 0.0, 1.0, 1.0}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 1.0}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 0.0}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
};

inline const std::vector<Vertex> pyramidVertices = {
    // Front
    {{ 0.0, 1.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 0.0}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5, 0.0, 0.5}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    // Back
    {{ 1.0, 1.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.0, 1.0, 1.0}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.5, 0.0, 0.5}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    // Left
    {{ 0.0, 1.0, 1.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.0, 1.0, 0.0}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.5, 0.0, 0.5}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    // Right
    {{ 1.0, 1.0, 0.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0, 1.0, 1.0}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.5, 0.0, 0.5}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    // Bottom
    {{ 0.0f, 1.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0f, 1.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0f, 1.0f,  0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 1.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
};

inline const std::vector<Vertex> lineVertices = {
	{{ 0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{ 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
};

inline const std::vector<Vertex> xAxisVertices = {
    {{ 0.0f,                   0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{ Constants::AXES_LENGTH, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
};

inline const std::vector<Vertex> yAxisVertices = {
    {{ 0.0f, 0.0f,                   0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{ 0.0f, Constants::AXES_LENGTH, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
};

inline const std::vector<Vertex> zAxisVertices = {
    {{ 0.0f, 0.0f,                   0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    {{ 0.0f, 0.0f, Constants::AXES_LENGTH}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
};