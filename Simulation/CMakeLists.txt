# CMakeList.txt : CMake project for VulkanCmakeTest, include source and define
# project specific logic here.
#

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows/share")

add_executable (Simulation 
    "extras.h"     
    "VulkanRenderer.cpp" 
    "VulkanRenderer.h" 
    "Simulation.cpp"
    "Simulation.h"
    "Camera.h"
    "main.cpp" 
    "PipelineFactory.h"
    "Vertex.h" 
    "Mesh.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Simulation PROPERTY CXX_STANDARD 20)
endif()

#External
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(Simulation PRIVATE glfw Vulkan::Vulkan)

target_include_directories(Simulation PRIVATE 
    "third_party/stb_image"
    "third_party/tiny_obj_loader"
)
