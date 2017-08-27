#pragma once

#include <limits>

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace lava
{
  /**
  * Types of texture addressing modes that determine what happens when texture coordinates are outside of the valid range.
  */
  enum class TextureAddressingMode
  {
    WRAP,     // Coordinates wrap back to the valid range.
    MIRROR,   // Coordinates flip every time the size of the valid range is passed.
    CLAMP,    // Coordinates are clamped within the valid range.
    BORDER    // Coordinates outside of the valid range will return a separately set border color.
  };

  // Texture addressing mode, per component.
  struct UVWAddressingMode
  {
    UVWAddressingMode( void )
      : u( TextureAddressingMode::WRAP )
      , v( TextureAddressingMode::WRAP )
      , w( TextureAddressingMode::WRAP )
    { }

    bool operator==(const UVWAddressingMode& rhs) const
    {
      return u == rhs.u && v == rhs.v && w == rhs.w;
    }

    TextureAddressingMode u, v, w;
  };

  // Filtering options for textures.
  enum class FilterOptions
  {
    NONE = 0,         // Use no filtering. Only relevant for mipmap filtering.
    POINT = 1,        // Filter using the nearest found pixel. Most basic filtering.
    LINEAR = 2,       // Average a 2x2 pixel area, signifies bilinear filtering for texture, trilinear for mipmaps.
    ANISOTROPIC = 3,  // More advanced filtering that improves quality when viewing textures at a steep angle
  };

  // Comparison functions used for the depth/stencil buffer.
  enum CompareFunction
  {
    ALWAYS_FAIL,    // Operation will always fail.
    ALWAYS_PASS,    // Operation will always pass.
    LESS,           // Operation will pass if the new value is less than existing value.
    LESS_EQUAL,     // Operation will pass if the new value is less or equal than existing value.
    EQUAL,          // Operation will pass if the new value is equal to the existing value.
    NOT_EQUAL,      // Operation will pass if the new value is not equal to the existing value.
    GREATER_EQUAL,  // Operation will pass if the new value greater or equal than the existing value.
    GREATER         // Operation will pass if the new value greater than the existing value.
  };

  struct SamplerStateDesc
  {
    SamplerStateDesc( void )
      : minFilter(FilterOptions::LINEAR)
      , magFilter(FilterOptions::LINEAR)
      , mipFilter(FilterOptions::LINEAR)
      , maxAniso(0)
      , mipmapBias(0)
      , mipMin(-FLT_MAX)
      , mipMax(FLT_MAX)/*,
      borderColor(Color::White)*/
      , comparisonFunc(CompareFunction::ALWAYS_PASS)
    { }

    // Determines how are texture coordinates outside of [0, 1] range handled.
    UVWAddressingMode addressMode;
    
    // Filtering used when texture is displayed as smaller than its original size.
    FilterOptions minFilter;
    
    // Filtering used when texture is displayed as larger than its original size.
    FilterOptions magFilter;
    
    // Filtering used to blend between the different mip levels.
    FilterOptions mipFilter; 

    // Maximum number of samples if anisotropic filtering is enabled. Max is 16.
    uint32_t maxAniso;

    /**
     * Mipmap bias allows you to adjust the mipmap selection calculation. Negative values  force a larger mipmap to be
     * used, and positive values smaller. Units are in values of mip levels, so -1 means use a mipmap one level higher 
     * than default.
     */
    float mipmapBias;

    // Minimum mip-map level that is allowed to be displayed.
    float mipMin;

    // Maximum mip-map level that is allowed to be displayed. Set to FLT_MAX for no limit.
    float mipMax;

    // Border color to use when using border addressing mode as specified by @p addressMode.
    //Color borderColor;

    // Function that compares sampled data with existing sampled data.
    CompareFunction comparisonFunc;
  };

  class Sampler : public VulkanResource, private NonCopyable<Sampler>
  {
  public:
	  Sampler(std::shared_ptr<Device> const & device,  const SamplerStateDesc& desc );
	  ~Sampler();

	  inline operator vk::Sampler() const
	  {
		  return _sampler;
	  }
  protected:
	  vk::Sampler _sampler;

    vk::Filter getFilter(FilterOptions filter)
    {
      switch(filter)
      {
      case FilterOptions::LINEAR:
      case FilterOptions::ANISOTROPIC:
        return vk::Filter::eLinear;
      case FilterOptions::POINT:
      case FilterOptions::NONE:
        return vk::Filter::eNearest;
      }

      // Unsupported type
      return vk::Filter::eLinear;
    }
	vk::SamplerMipmapMode getMipFilter(FilterOptions filter)
    {
      switch (filter)
      {
      case FilterOptions::LINEAR:
      case FilterOptions::ANISOTROPIC:
        return vk::SamplerMipmapMode::eLinear;
      case FilterOptions::POINT:
      case FilterOptions::NONE:
        return vk::SamplerMipmapMode::eNearest;
      }

      // Unsupported type
      return vk::SamplerMipmapMode::eLinear;
    }
	vk::SamplerAddressMode getAddressingMode(TextureAddressingMode mode)
    {
      switch (mode)
      {
      case TextureAddressingMode::WRAP:
        return vk::SamplerAddressMode::eRepeat;
      case TextureAddressingMode::MIRROR:
        return vk::SamplerAddressMode::eMirroredRepeat;
      case TextureAddressingMode::CLAMP:
        return vk::SamplerAddressMode::eClampToEdge;
      case TextureAddressingMode::BORDER:
        return vk::SamplerAddressMode::eClampToBorder;
      }

      return vk::SamplerAddressMode::eRepeat;
    }
	vk::CompareOp getCompareOp(CompareFunction op)
    {
      switch (op)
      {
      case CompareFunction::ALWAYS_FAIL:
        return vk::CompareOp::eNever;
      case CompareFunction::ALWAYS_PASS:
        return vk::CompareOp::eAlways;
      case CompareFunction::LESS:
        return vk::CompareOp::eLess;
      case CompareFunction::LESS_EQUAL:
        return vk::CompareOp::eLessOrEqual;
      case CompareFunction::EQUAL:
        return vk::CompareOp::eEqual;
      case CompareFunction::NOT_EQUAL:
        return vk::CompareOp::eNotEqual;
      case CompareFunction::GREATER_EQUAL:
        return vk::CompareOp::eGreaterOrEqual;
      case CompareFunction::GREATER:
        return vk::CompareOp::eGreater;
      };

      // Unsupported type
      return vk::CompareOp::eAlways;
    }
  };
}