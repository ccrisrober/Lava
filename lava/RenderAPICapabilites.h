/**
 * Copyright (c) 2017 - 2018, Lava
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

#ifndef __LAVA_RENDERAPICAPABILITIES_
#define __LAVA_RENDERAPICAPABILITIES_

#include <algorithm>

#include <vector>
#include <set>
#include <map>

#include <sstream>

#include <cstdint>
#define CAPS_CATEGORY_SIZE INT64_C(8)
#define LV_CAPS_BITSHIFT (INT64_C(64) - CAPS_CATEGORY_SIZE)
#define CAPS_CATEGORY_MASK (((INT64_C(1) << CAPS_CATEGORY_SIZE) - INT64_C(1)) << LV_CAPS_BITSHIFT)
#define LV_CAPS_VALUE(cat, val) ((cat << LV_CAPS_BITSHIFT) | (INT64_C(1) << val))

#define LV_MAX_BOUND_VERTEX_BUFFERS 16

namespace lava
{
  // Categories of render API capabilities.
  enum CapabilitiesCategory
  {
    CAPS_CATEGORY_COMMON = 0,
    CAPS_CATEGORY_GL = 1,
    CAPS_CATEGORY_D3D11 = 2,
    CAPS_CATEGORY_VULKAN = 3,
    CAPS_CATEGORY_COUNT = 32 // Maximum number of categories.
  };
  enum Capabilities : short
  {
    // Supports compressed textures in the BC formats.
    RSC_TEXTURE_COMPRESSION_BC = 0,
    // Supports compressed textures in the ETC2 and EAC format.
    RSC_TEXTURE_COMPRESSION_ETC2 = 1,
    // Supports compressed textures in the ASTC format.
    RSC_TEXTURE_COMPRESSION_ASTC = 2,
    // Supports geometry shaders.
    RSC_GEOMETRY_PROGRAM = 3,
    // Supports tessellation shaders.
    RSC_TESSELLATION_PROGRAM = 4,
    // Supports compute shaders.
    RSC_COMPUTE_PROGRAM = 5,
  };

  struct DriverVersion
  {
    std::string toString( void ) const
    {
      std::stringstream str;
      str << major << "." << minor << "." << release << "." << build;
      return str.str( );
    }

    int32_t major = 0;
    int32_t minor = 0;
    int32_t release = 0;
    int32_t build = 0;
  };

  // Types of GPU vendors.
  enum GPUVendor
  {
    GPU_UNKNOWN = 0,
    GPU_NVIDIA = 1,
    GPU_AMD = 2,
    GPU_INTEL = 3,
    GPU_VENDOR_COUNT = 4
  };
  enum class GpuProgramType
  {
    VERTEX_PROGRAM,       // Vertex program
    FRAGMENT_PROGRAM,     // Fragment(pixel) program
    GEOMETRY_PROGRAM,     // Geometry program
    TESS_EVAL_PROGRAM,    // Tesselation evaluation program
    TESS_CTRL_PROGRAM,    // Tesselation control program
    COMPUTE_PROGRAM,      // Compute program.
    COUNT
  };
  /**
  * Holds information about render hardware and driver capabilities
  * and allows you to easily set and query those capabilities.
  */
  class RenderAPICapabilities
  {
  public:
    RenderAPICapabilities( void )
    {
      for ( uint32_t i = 0; i < CAPS_CATEGORY_COUNT; ++i )
      {
        _capabilities[ i ] = 0;
      }

      _numTextureUnitsPerStage[ GpuProgramType::VERTEX_PROGRAM ] = 0;
      _numTextureUnitsPerStage[ GpuProgramType::FRAGMENT_PROGRAM ] = 0;
      _numTextureUnitsPerStage[ GpuProgramType::GEOMETRY_PROGRAM ] = 0;
      _numTextureUnitsPerStage[ GpuProgramType::TESS_EVAL_PROGRAM ] = 0;
      _numTextureUnitsPerStage[ GpuProgramType::TESS_CTRL_PROGRAM ] = 0;
      _numTextureUnitsPerStage[ GpuProgramType::COMPUTE_PROGRAM ] = 0;

      _numGpuParamBlocksPerStage[ GpuProgramType::VERTEX_PROGRAM ] = 0;
      _numGpuParamBlocksPerStage[ GpuProgramType::FRAGMENT_PROGRAM ] = 0;
      _numGpuParamBlocksPerStage[ GpuProgramType::GEOMETRY_PROGRAM ] = 0;
      _numGpuParamBlocksPerStage[ GpuProgramType::TESS_EVAL_PROGRAM ] = 0;
      _numGpuParamBlocksPerStage[ GpuProgramType::TESS_CTRL_PROGRAM ] = 0;
      _numGpuParamBlocksPerStage[ GpuProgramType::COMPUTE_PROGRAM ] = 0;

      _numLoadStoreTextureUnitsPerStage[ GpuProgramType::VERTEX_PROGRAM ] = 0;
      _numLoadStoreTextureUnitsPerStage[ GpuProgramType::FRAGMENT_PROGRAM ] = 0;
      _numLoadStoreTextureUnitsPerStage[ GpuProgramType::GEOMETRY_PROGRAM ] = 0;
      _numLoadStoreTextureUnitsPerStage[ GpuProgramType::TESS_EVAL_PROGRAM ] = 0;
      _numLoadStoreTextureUnitsPerStage[ GpuProgramType::TESS_CTRL_PROGRAM ] = 0;
      _numLoadStoreTextureUnitsPerStage[ GpuProgramType::COMPUTE_PROGRAM ] = 0;
    }

    ~RenderAPICapabilities( void )
    { }

    // Sets the current driver version.
    void setDriverVersion( const DriverVersion& version )
    {
      _driverVersion = version;
    }

    // Returns current driver version.
    DriverVersion getDriverVersion( void ) const
    {
      return _driverVersion;
    }

    // Returns vendor of the currently used GPU.
    GPUVendor getVendor( void ) const
    {
      return _vendor;
    }

    // Sets the GPU vendor.
    void setVendor( GPUVendor v )
    {
      _vendor = v;
    }

    /** Parses a vendor string and returns an enum with the vendor if
     * parsed succesfully.
     */
    static GPUVendor vendorFromString( const std::string& vendorString )
    {
      initVendorStrings( );
      GPUVendor ret = GPU_UNKNOWN;
      std::string cmpString = vendorString;
      std::transform( cmpString.begin( ), cmpString.end( ),
        cmpString.begin( ), ::tolower );
      for ( int i = 0; i < GPU_VENDOR_COUNT; ++i )
      {
        // case insensitive (lower case)
        if ( _gpuVendorStrings[ i ] == cmpString )
        {
          ret = static_cast< GPUVendor >( i );
          break;
        }
      }

      return ret;
    }

    // Converts a vendor enum to a string.
    static std::string vendorToString( GPUVendor v )
    {
      initVendorStrings( );
      return _gpuVendorStrings[ v ];
    }

    // Sets the maximum number of texture units per pipeline stage.
    void setNumTextureUnits( GpuProgramType type, uint16_t num )
    {
      _numTextureUnitsPerStage[ type ] = num;
    }

    // Sets the maximum number of texture units in all pipeline stages.
    void setNumCombinedTextureUnits( uint16_t num )
    {
      _numCombinedTextureUnits = num;
    }

    // Sets the maximum number of load-store texture units per pipeline stage.
    void setNumLoadStoreTextureUnits( GpuProgramType type, uint16_t num )
    {
      _numLoadStoreTextureUnitsPerStage[ type ] = num;
    }

    // Sets the maximum number of load-store texture units in all pipeline stages.
    void setNumCombinedLoadStoreTextureUnits( uint16_t num )
    {
      _numCombinedLoadStoreTextureUnits = num;
    }

    // Sets the maximum number of GPU param block buffers per pipeline stage.
    void setNumGpuParamBlockBuffers( GpuProgramType type, uint16_t num )
    {
      _numGpuParamBlocksPerStage[ type ] = num;
    }

    // Sets the maximum number of GPU parammN block buffers in all pipeline stages.
    void setNumCombinedGpuParamBlockBuffers( uint16_t num )
    {
      _numCombinedUniformBlocks = num;
    }

    // Sets maximum number of bound vertex buffers.
    void setMaxBoundVertexBuffers( uint32_t num )
    {
      _maxBoundVertexBuffers = num;
    }

    // Sets maximum number of simultaneously set render targets.
    void setNumMultiRenderTargets( uint16_t num )
    {
      _numMultiRenderTargets = num;
    }

    // Returns the number of texture units supported per pipeline stage.
    uint16_t getNumTextureUnits( GpuProgramType type ) const
    {
      auto iterFind = _numTextureUnitsPerStage.find( type );
      if ( iterFind != _numTextureUnitsPerStage.end( ) )
        return iterFind->second;
      else
        return 0;
    }

    // Returns the number of texture units supported in all pipeline stages.
    uint16_t getNumCombinedTextureUnits( void ) const
    {
      return _numCombinedTextureUnits;
    }

    // Returns the number of load-store texture units supported per pipeline stage.
    uint16_t getNumLoadStoreTextureUnits( GpuProgramType type ) const
    {
      auto iterFind = _numLoadStoreTextureUnitsPerStage.find( type );
      if ( iterFind != _numLoadStoreTextureUnitsPerStage.end( ) )
        return iterFind->second;
      else
        return 0;
    }

    // Returns the number of load-store texture units supported in all pipeline stages.
    uint16_t getNumCombinedLoadStoreTextureUnits( void ) const
    {
      return _numCombinedLoadStoreTextureUnits;
    }

    // Returns the maximum number of bound GPU program param block buffers per pipeline stage.
    uint16_t getNumGpuParamBlockBuffers( GpuProgramType type ) const
    {
      auto iterFind = _numGpuParamBlocksPerStage.find( type );
      if ( iterFind != _numGpuParamBlocksPerStage.end( ) )
        return iterFind->second;
      else
        return 0;
    }

    // Returns the maximum number of bound GPU program param block buffers in all pipeline stages.
    uint16_t getNumCombinedGpuParamBlockBuffers( void ) const
    {
      return _numCombinedUniformBlocks;
    }

    // Returns the maximum number of vertex buffers that can be bound at once.
    uint32_t getMaxBoundVertexBuffers( void ) const
    {
      return _maxBoundVertexBuffers;
    }

    // Returns the maximum number of render targets we can render to simultaneously.
    uint16_t getNumMultiRenderTargets( void ) const
    {
      return _numMultiRenderTargets;
    }

    // Sets a capability flag indicating this capability is supported.
    void setCapability( const Capabilities c )
    {
      uint64_t index = ( CAPS_CATEGORY_MASK & c ) >> LV_CAPS_BITSHIFT;
      _capabilities[ index ] |= ( c & ~CAPS_CATEGORY_MASK );
    }

    // Remove a capability flag indicating this capability is not supported (default).
    void unsetCapability( const Capabilities c )
    {
      uint64_t index = ( CAPS_CATEGORY_MASK & c ) >> LV_CAPS_BITSHIFT;
      _capabilities[ index ] &= ( ~c | CAPS_CATEGORY_MASK );
    }

    // Checks is the specified capability supported.
    bool hasCapability( const Capabilities c ) const
    {
      uint64_t index = ( CAPS_CATEGORY_MASK & c ) >> LV_CAPS_BITSHIFT;

      return ( _capabilities[ index ] & ( c & ~CAPS_CATEGORY_MASK ) ) != 0;
    }

    // Adds a shader profile to the list of render-system specific supported profiles.
    void addShaderProfile( const std::string& profile )
    {
      _supportedShaderProfiles.insert( profile );
    }

    // Returns true if the provided profile is supported.
    bool isShaderProfileSupported( const std::string& profile ) const
    {
      return ( _supportedShaderProfiles.end( ) != _supportedShaderProfiles.find( profile ) );
    }

    // Returns a set of all supported shader profiles.
    const std::set<std::string>& getSupportedShaderProfiles( ) const
    {
      return _supportedShaderProfiles;
    }

    // Sets the current GPU device name.
    void setDeviceName( const std::string& name )
    {
      _deviceName = name;
    }

    // Gets the current GPU device name.
    std::string getDeviceName( void ) const
    {
      return _deviceName;
    }

    // Sets the number of vertices a single geometry program run can emit.
    void setGeometryProgramNumOutputVertices( int numOutputVertices )
    {
      _geometryProgramNumOutputVertices = numOutputVertices;
    }

    // Gets the number of vertices a single geometry program run can emit.
    int getGeometryProgramNumOutputVertices( void ) const
    {
      return _geometryProgramNumOutputVertices;
    }
  private:
    // Initializes vendor enum -> vendor name mappings.
    static void initVendorStrings( void )
    {
      if ( _gpuVendorStrings.empty( ) )
      {
        // Always lower case
        _gpuVendorStrings.resize( GPU_VENDOR_COUNT );
        _gpuVendorStrings[ GPU_UNKNOWN ] = "unknown";
        _gpuVendorStrings[ GPU_NVIDIA ] = "nvidia";
        _gpuVendorStrings[ GPU_AMD ] = "amd";
        _gpuVendorStrings[ GPU_INTEL ] = "intel";
      }
    }

  private:
    static std::vector<std::string> _gpuVendorStrings;

    DriverVersion _driverVersion;
    GPUVendor _vendor = GPU_UNKNOWN;

    // The number of texture units available per stage
    std::map<GpuProgramType, uint16_t> _numTextureUnitsPerStage;
    // Total number of texture units available
    uint16_t _numCombinedTextureUnits = 0;
    // The number of uniform blocks available per stage
    std::map<GpuProgramType, uint16_t> _numGpuParamBlocksPerStage;
    // Total number of uniform blocks available
    uint16_t _numCombinedUniformBlocks = 0;
    // The number of load-store texture unitss available per stage
    std::map<GpuProgramType, uint16_t> _numLoadStoreTextureUnitsPerStage;
    // Total number of load-store texture units available
    uint16_t _numCombinedLoadStoreTextureUnits = 0;
    // Maximum number of vertex buffers we can bind at once
    uint32_t _maxBoundVertexBuffers = 0;
    // Stores the capabilities flags.
    uint32_t _capabilities[ CAPS_CATEGORY_COUNT ];
    // The name of the device as reported by the render system
    std::string _deviceName;

    // The number of simultaneous render targets supported
    uint16_t _numMultiRenderTargets = 0;
    // The number of vertices a geometry program can emit in a single run
    uint32_t _geometryProgramNumOutputVertices = 0;

    // The list of supported shader profiles
    std::set<std::string> _supportedShaderProfiles;
  };
}

#endif /* __LAVA_RENDERAPICAPABILITIES_ */