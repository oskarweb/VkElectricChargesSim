#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>
#include <array>
#include <vector>
#include <filesystem>

#include "extras.h"
#include "Camera.h"


//(constants::MODELS_PATH / "viking_room.obj").string().c_str()

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

template<>
struct std::hash<Vertex> {
	size_t operator()(Vertex const& vertex) const {
		return ((std::hash<glm::vec3>()(vertex.pos) ^
			(std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
			(std::hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};

struct Mesh 
{
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;
	std::unordered_map<Vertex, uint32_t> m_uniqueVertices{};

	VkDevice m_deviceHandle = VK_NULL_HANDLE;
	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

	bool loadFromObj(std::filesystem::path& path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str()))
		{
			return false;
		}

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (m_uniqueVertices.count(vertex) == 0) {
					m_uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
					m_vertices.emplace_back(vertex);
				}

				m_indices.emplace_back(m_uniqueVertices[vertex]);
			}
		}
		return true;
	}

	void upload(VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool)
	{
		createVertexBuffer(physicalDevice, graphicsQueue, commandPool);
		createIndexBuffer(physicalDevice, graphicsQueue, commandPool);
	}

	void createIndexBuffer(VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool)
	{
		VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		helpers::createBuffer(
			m_deviceHandle,
			physicalDevice,
			bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer, 
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(m_deviceHandle, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_deviceHandle, stagingBufferMemory);

		helpers::createBuffer(
			m_deviceHandle,
			physicalDevice,
			bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_indexBuffer,
			m_indexBufferMemory
		);

		helpers::copyBuffer(m_deviceHandle, graphicsQueue, commandPool, stagingBuffer, m_indexBuffer, bufferSize);

		vkDestroyBuffer(m_deviceHandle, stagingBuffer, nullptr);
		vkFreeMemory(m_deviceHandle, stagingBufferMemory, nullptr);
	}

	void createVertexBuffer(VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool commandPool)
	{
		VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		helpers::createBuffer(
			m_deviceHandle,
			physicalDevice,
			bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			stagingBuffer,
			stagingBufferMemory
		);

		void* data;
		vkMapMemory(m_deviceHandle, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, m_vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_deviceHandle, stagingBufferMemory);

		helpers::createBuffer(
			m_deviceHandle,
			physicalDevice,
			bufferSize, 
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_vertexBuffer,
			m_vertexBufferMemory
		);

		helpers::copyBuffer(m_deviceHandle, graphicsQueue, commandPool, stagingBuffer, m_vertexBuffer, bufferSize);

		vkDestroyBuffer(m_deviceHandle, stagingBuffer, nullptr);
		vkFreeMemory(m_deviceHandle, stagingBufferMemory, nullptr);
	}

	Mesh() = default;
	Mesh(VkDevice device) {
		m_deviceHandle = device;
	}

	void cleanup() {
		if (m_indexBuffer != VK_NULL_HANDLE)
			vkDestroyBuffer(m_deviceHandle, m_indexBuffer, nullptr);
		if (m_indexBufferMemory != VK_NULL_HANDLE)
			vkFreeMemory(m_deviceHandle, m_indexBufferMemory, nullptr);
		if (m_vertexBuffer != VK_NULL_HANDLE)
			vkDestroyBuffer(m_deviceHandle, m_vertexBuffer, nullptr);
		if (m_vertexBufferMemory != VK_NULL_HANDLE)
			vkFreeMemory(m_deviceHandle, m_vertexBufferMemory, nullptr);
	}
};

struct Material 
{
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct Renderable 
{
	Mesh* mesh;
	Material* material;

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

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class VulkanRenderer 
{
public:
	const static int MAX_FRAMES_IN_FLIGHT = 2;

	void setCamera(Camera* camera)
	{
		m_cameraPtr = camera;
	}

	void setWindow(GLFWwindow* window)
	{
		m_window = window;
		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}
	
	void init();
	void drawFrame();
	void cleanup();

private:
	void mainLoop();
	void cleanupSwapChain();
	void recreateSwapChain();
	void createInstance();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline(VkPipeline& pipeline, VkPipelineLayout& pipelineLayout, std::vector<char>&& vertShaderCode, std::vector<char>&& fragShaderCode);
	void createFramebuffers();
	void createCommandPool();
	void createColorResources();
	void createDepthResources();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat();
	void createTextureImage(VkImage& textureImage, VkDeviceMemory& textureImageMemory, std::filesystem::path& path);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	void createTextureImageView(VkImageView& textureImageView, VkImage& textureImage);
	void createTextureSampler();
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	//void loadModel();
	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void createCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void createSyncObjects();
	void updateUniformBuffer(uint32_t currentImage);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);
	Material* getMaterial(const std::string& name);
	Mesh* getMesh(const std::string& name);
	void drawObjects(VkCommandBuffer& commandBuffer, Renderable* objects, size_t count);

	//VARS
	std::vector<Renderable> m_renderableObjects;
	std::unordered_map<std::string, Mesh> m_meshes;
	std::unordered_map<std::string, Material> m_materials;

	std::vector<Mesh> testMesh;

	Camera* m_cameraPtr = nullptr;
	GLFWwindow* m_window = nullptr;
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	VkSurfaceKHR m_surface;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	VkFormat m_swapChainImageFormat;
	VkExtent2D m_swapChainExtent;
	std::vector<VkImageView> m_swapChainImageViews;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;

	VkRenderPass m_renderPass;
	VkDescriptorSetLayout m_descriptorSetLayout;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;

	VkCommandPool m_commandPool;
	std::vector<VkCommandBuffer> m_commandBuffers;

	std::vector<VkSemaphore> m_imageAvailableSemaphores;
	std::vector<VkSemaphore> m_renderFinishedSemaphores;
	std::vector<VkFence> m_inFlightFences;

	uint32_t m_currentFrame = 0;

	//std::unordered_map<Vertex, uint32_t> m_uniqueVertices{};
	//std::vector<Vertex> m_vertices;
	//std::vector<uint32_t> m_indices;
	
	//VkBuffer m_vertexBuffer;
	//VkDeviceMemory m_vertexBufferMemory;

	//VkBuffer m_indexBuffer;
	//VkDeviceMemory m_indexBufferMemory;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;

	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;

	uint32_t m_mipLevels;
	VkSampler m_textureSampler;

	VkImage m_textureImage;
	VkDeviceMemory m_textureImageMemory;
	VkImageView m_textureImageView;

	VkImage m_depthImage;
	VkDeviceMemory m_depthImageMemory;
	VkImageView m_depthImageView;

	VkImage m_colorImage;
	VkDeviceMemory m_colorImageMemory;
	VkImageView m_colorImageView;

	VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	bool m_framebufferResized = false;
};

inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

inline bool hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};