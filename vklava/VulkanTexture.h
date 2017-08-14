#pragma once

#include <vulkan/vulkan.h>

#include "VulkanResource.h"

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

struct VULKAN_IMAGE_DESC
{
  VkImage image; /**< Internal Vulkan image object */
  VkDeviceMemory memory; /**< Memory bound to the image. */
  VkImageLayout layout; /**< Initial layout of the image. */
  TextureType type; /**< Type of the image. */
  VkFormat format; /**< Pixel format of the image. */
  uint32_t numFaces; /**< Number of faces (array slices, or cube-map faces). */
  uint32_t numMipLevels; /**< Number of mipmap levels per face. */
  TextureUsage usage; /** Determines how will the image be used. */
};

class VulkanImage : public VulkanResource
{
public:
  VulkanImage( VulkanDevicePtr device, VkImage image, VkDeviceMemory mem,
    VkImageLayout layout )
  : VulkanResource( device )
  {
  }
};

class VulkanTexture : public VulkanResource
{
public:
  VulkanTexture( VulkanDevicePtr device )
    : VulkanResource( device )
  {
  }
};
