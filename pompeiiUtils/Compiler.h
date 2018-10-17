/**
 * Copyright (c) 2017 - 2018, Pompeii
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __POMPEIIUTILS_COMPILER__
#define __POMPEIIUTILS_COMPILER__

#include <pompeii/pompeii.h>
#include <pompeiiUtils/api.h>
#include <vector>

namespace pompeii
{
  namespace utils
  {
    POMPEIIUTILS_API
    bool GLSLtoSPV( vk::ShaderStageFlagBits stage,
      const std::string& source, std::vector<uint32_t> &spirv );
  }
}

#endif /* __POMPEIIUTILS_COMPILER__  */