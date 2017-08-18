#ifndef __VKLAVA_VULKANTEXTURE__
#define __VKLAVA_VULKANTEXTURE__

#include <vulkan/vulkan.h>

#include "VulkanResource.h"

namespace lava
{
  enum TextureUsage : uint32_t
  {
    /** A regular texture that is not often or ever updated from the CPU. */
    TU_STATIC,
    /** A regular texture that is often updated by the CPU. */
    TU_DYNAMIC,
    /** Texture that can be rendered to by the GPU. */
    TU_RENDERTARGET,
    /** Texture used as a depth/stencil buffer by the GPU. */
    TU_DEPTHSTENCIL,
    /** Texture that allows load/store operations from the GPU program. */
    TU_LOADSTORE,
    /** All mesh data will also be cached in CPU memory, making it available for fast read access from the CPU. */
    TU_CPUCACHED,
    /** Allows the CPU to directly read the texture data buffers from the GPU. */
    TU_CPUREADABLE,
    /** Default (most common) texture usage. */
    TU_DEFAULT
  };

  enum TextureType
  {
    /** One dimensional texture. Just a row of pixels. */
    TEX_TYPE_1D = 1,
    /** Two dimensional texture. */
    TEX_TYPE_2D = 2,
    /** Three dimensional texture. */
    TEX_TYPE_3D = 3,
    /** Texture consisting out of six 2D textures describing an inside of a cube. Allows special sampling. */
    TEX_TYPE_CUBE_MAP = 4
  };

  class VulkanImage : public VulkanResource
  {
  public:
    VulkanImage( VulkanDevicePtr device, VkImage image, VkDeviceMemory mem,
      VkImageLayout layout )
      : VulkanResource( device )
    { }
  };
  enum PixelFormat
  {
    /** Unknown pixel format. */
    PF_UNKNOWN,
    /** 8-bit pixel format, all bits red. */
    PF_R8,
    /** 2 byte pixel format, 1 byte red, 1 byte green. */
    PF_R8G8,
    /** 24-bit pixel format, 8 bits for red, green and blue. */
    PF_R8G8B8,
    /** 24-bit pixel format, 8 bits for blue, green and red. */
    PF_B8G8R8,
    /** 32-bit pixel format, 8 bits for blue, green, red and alpha. */
    PF_B8G8R8A8,
    /** 32-bit pixel format, 8 bits for red, green, blue and alpha. */
    PF_R8G8B8A8
  };
  struct TEXTURE_DESC
  {
    /** Type of the texture. */
    TextureType type = TEX_TYPE_2D;

    /** Format of pixels in the texture. */
    PixelFormat format = PF_R8G8B8A8;

    /** Width of the texture in pixels. */
    uint32_t width = 1;

    /** Height of the texture in pixels. */
    uint32_t height = 1;

    /** Depth of the texture in pixels (Must be 1 for 2D textures). */
    uint32_t depth = 1;

    /** Number of mip-maps the texture has. This number excludes the full resolution map. */
    uint32_t numMips = 0;

    /** Describes how the caller plans on using the texture in the pipeline. */
    int32_t usage = TU_DEFAULT;

    /**
    * If true the texture data is assumed to have been gamma corrected and will be converted back to linear space when
    * sampled on GPU.
    */
    bool hwGamma = false;

    /** Number of samples per pixel. Set to 1 or 0 to use the default of a single sample per pixel. */
    uint32_t numSamples = 0;

    /** Number of texture slices to create if creating a texture array. Ignored for 3D textures. */
    uint32_t numArraySlices = 1;
  };
  class TextureProperties
  {
  public:
    TextureProperties::TextureProperties( )
    { }

    TextureProperties::TextureProperties( const TEXTURE_DESC& desc )
      :mDesc( desc )
    {
    }

    /**	Gets the type of texture. */
    TextureType getTextureType( ) const
    {
      return mDesc.type;
    }

    /**
    * Gets the number of mipmaps to be used for this texture. 
    * This number excludes the top level map (which is always assumed to be present).
    */
    uint32_t getNumMipmaps( ) const
    {
      return mDesc.numMips;
    }

    /**
    * Determines does the texture contain gamma corrected data. 
    * If true then the GPU will automatically convert the pixels to linear 
    * space before reading from the texture, and convert them to gamma space 
    * when writing to the texture.
    */
    bool isHardwareGammaEnabled( ) const
    {
      return mDesc.hwGamma;
    }

    /**	Gets the number of samples used for multisampling (0 or 1 if 
    * multisampling is not used).
    */
    uint32_t getNumSamples( ) const
    {
      return mDesc.numSamples;
    }

    //	Returns the height of the texture.
    uint32_t getHeight( ) const
    {
      return mDesc.height;
    }

    //	Returns the width of the texture.
    uint32_t getWidth( ) const
    {
      return mDesc.width;
    }

    //	Returns the depth of the texture (only applicable for 3D textures).
    uint32_t getDepth( ) const
    {
      return mDesc.depth;
    }

    /**
     * Returns a value that signals the engine in what way is the texture 
     * expected to be used.
     */
    int getUsage( ) const
    {
      return mDesc.usage;
    }

    //	Returns the pixel format for the texture surface.
    PixelFormat getFormat( ) const
    {
      return mDesc.format;
    }

    //	Returns true if the texture has an alpha layer.
    bool hasAlpha( ) const;

    /**
    * Returns the number of faces this texture has. This includes array slices 
    * (if texture is an array texture), as well as cube-map faces.
    */
    uint32_t getNumFaces( ) const;

    /** Returns the number of array slices of the texture 
    * (if the texture is an array texture).
    */
    uint32_t getNumArraySlices( ) const
    {
      return mDesc.numArraySlices;
    }
  protected:
    TEXTURE_DESC mDesc;
  };
  struct VULKAN_IMAGE_DESC
  {
    VkImage image;          // Internal Vulkan image object
    VkDeviceMemory memory;  // Memory bound to the image.
    VkImageLayout layout;   // Initial layout of the image.
    TextureType type;       // Type of the image.
    VkFormat format;        // Pixel format of the image.
    uint32_t numFaces;      // Number of faces (array slices, or cube-map faces).
    uint32_t numMipLevels;  // Number of mipmap levels per face.
    uint32_t usage;         // Determines how will the image be used.
  };
  class VulkanTexture : public VulkanResource
  {
  public:
    VulkanTexture( VulkanDevicePtr device, const VULKAN_IMAGE_DESC& desc )
      : VulkanResource( device )
      , mImage( desc.image )
      , mMemory( desc.memory )
      , mFramebufferMainView( VK_NULL_HANDLE )
      //, mOwnsImage( ownsImage )
      , mNumFaces( desc.numFaces )
      , mNumMipLevels( desc.numMipLevels )
      , mUsage( desc.usage )
    {
      _imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      _imageViewCI.pNext = nullptr;
      _imageViewCI.flags = 0;
      _imageViewCI.image = desc.image;
      _imageViewCI.format = desc.format;
      _imageViewCI.components = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A
      };
      switch ( desc.type )
      {
        case TEX_TYPE_1D:
          _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_1D;
          break;
        default:
        case TEX_TYPE_2D:
          _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
          break;
        case TEX_TYPE_3D:
          _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_3D;
          break;
        case TEX_TYPE_CUBE_MAP:
          _imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
          break;
      }
    }
    VulkanTexture( VulkanDevicePtr device, VkImage image, VkDeviceMemory mem, 
      VkImageLayout layout, const TextureProperties& props )
      : VulkanTexture( device, createDesc( image, mem, layout, props ) )
    {
    }
    VkFormat getPixelFormat( PixelFormat format, bool sRGB )
    {
      switch ( format )
      {
        case PF_R8G8B8A8:
          if ( sRGB )
            return VK_FORMAT_R8G8B8A8_SRGB;

          return VK_FORMAT_R8G8B8A8_UNORM;
        case PF_UNKNOWN:
        default:
          return VK_FORMAT_UNDEFINED;
      }
    }
    VULKAN_IMAGE_DESC createDesc( VkImage image, VkDeviceMemory memory, 
      VkImageLayout layout, const TextureProperties& props )
    {
      VULKAN_IMAGE_DESC desc;
      desc.image = image;
      desc.memory = memory;
      desc.type = props.getTextureType( );
      desc.format = getPixelFormat( props.getFormat( ), props.isHardwareGammaEnabled( ) );
      desc.numFaces = props.getNumFaces( );
      desc.numMipLevels = props.getNumMipmaps( ) + 1;
      desc.layout = layout;
      desc.usage = ( uint32_t ) props.getUsage( );

      return desc;
    }
    VkImage mImage;
    VkDeviceMemory mMemory;
    VkImageView mMainView;
    VkImageView mFramebufferMainView;
    int32_t mUsage;
    VkImageViewCreateInfo _imageViewCI;
    uint32_t mNumFaces;
    uint32_t mNumMipLevels;
  };
}

#endif /* __VKLAVA_VULKANTEXTURE__ */