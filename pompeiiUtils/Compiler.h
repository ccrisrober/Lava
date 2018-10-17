#pragma once

#include <pompeii/pompeii.h>
#include <pompeiiUtils/api.h>

#include <glslang/SPIRV/GlslangToSpv.h>

namespace pompeii
{
  namespace utility
  {
    POMPEIIUTILS_API
    bool GLSLtoSPV( vk::ShaderStageFlagBits shaderType, const char* pshader,
      std::vector<uint32_t>& spirv );
  }
}