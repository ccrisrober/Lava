#ifndef __LAVA_INCLUDES__
#define __LAVA_INCLUDES__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#define LAVA_RUNTIME_ERROR(s) throw std::runtime_error( s );

#endif /* __LAVA_INCLUDES__ */