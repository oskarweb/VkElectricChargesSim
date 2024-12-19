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
	std::unique_ptr<int> textureIdx = nullptr;
	uint64_t id = 0;

	glm::mat4 transformMatrix;
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