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
    "Particle.cpp"
    "Particle.h"
    "RendererStructs.h"
    "Input.h"
    "Models.h"
    "Node.cpp"
    "Node.h"
    "Renderer.h"
    "Trail.cpp" 
    "Trail.h"
    "ChargedCuboid.cpp"
    "ChargedCuboid.h"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Simulation PROPERTY CXX_STANDARD 20)
endif()

#External
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_definitions(
    -DGLM_FORCE_CTOR_INIT
    -DGLM_FORCE_DEPTH_ZERO_TO_ONE
    -DGLM_FORCE_LEFT_HANDED
    -DGLM_FORCE_RADIANS
    -DGLM_FORCE_DEFAULT_ALIGNED_GENTYPES
    -DGLM_ENABLE_EXPERIMENTAL
)

target_link_libraries(Simulation PRIVATE glfw Vulkan::Vulkan imgui::imgui glm::glm)

target_include_directories(Simulation PRIVATE 
    "third_party/stb_image"
    "third_party/tiny_obj_loader"
    "third_party/implot"
)

target_sources(Simulation PRIVATE 
    "third_party/stb_image/stb_image.h"
    "third_party/tiny_obj_loader/tiny_obj_loader.h"
    "third_party/implot/implot.cpp"
    "third_party/implot/implot_items.cpp"
    "third_party/implot/implot_internal.h"
    "third_party/implot/implot_demo.cpp"
    "third_party/implot/implot.h"
)