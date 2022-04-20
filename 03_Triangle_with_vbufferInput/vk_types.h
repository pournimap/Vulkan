#pragma once

#define VK_USE_PLATFORM_WIN32_KHR 


//we will add basic types such as vertex definations
#include <vulkan/vulkan.h>
#include "vulkan/vk_sdk_platform.h"
#include <assert.h>
#include <vector>
#include <set>
#include <string>

//swapChain
#include <cstdint> //for uint32_t

#include <limits> //std::numeric_limits
#include <algorithm> //for std::clamp

#include <stdexcept>
#include <optional> //c++17
#include "vmath.h"
using namespace vmath;

#include <fstream>

const int MAX_FRAMES_IN_FLIGHT = 2;

