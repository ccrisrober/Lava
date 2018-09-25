#pragma once

#include <pompeii/pompeii.h>
#include <pompeiiUtils/api.h>

namespace pompeii
{
  namespace utility
  {
    POMPEIIUTILS_API
    std::vector<uint32_t> compileGLSLToSPIRV(vk::ShaderStageFlagBits stage, 
      std::string const & source);
  }
}