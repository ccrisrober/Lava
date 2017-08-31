#ifndef __LAVA_DEVICE__
#define __LAVA_DEVICE__

#include "includes.hpp"
#include "Semaphore.h"
#include "Queue.h"
#include "RenderPass.h"
#include "Swapchain.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"
#include "Sampler.h"
#include "Descriptor.h"

#include <vector>
#include <map>

#include "noncopyable.hpp"

namespace lava
{
  uint32_t findMemoryType( const vk::PhysicalDeviceMemoryProperties& memoryProperties, uint32_t requirementBits,
    vk::MemoryPropertyFlags wantedFlags );
  class PhysicalDevice;
  class Device : private NonCopyable<Device>, public std::enable_shared_from_this<Device>
  {
  public:
    static DeviceRef create( const std::shared_ptr<PhysicalDevice>& phyDev,
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures );
    Device( const std::shared_ptr<PhysicalDevice>& phyDev );
    virtual ~Device( );
    inline operator vk::Device( ) const
    {
      return _device;
    }

    void waitIdle( void )
    {
      _device.waitIdle( );
    }
    std::shared_ptr<Semaphore> createSemaphore( void );

    std::shared_ptr<RenderPass> createRenderPass(
      vk::ArrayProxy<const vk::AttachmentDescription> attachments,
      vk::ArrayProxy<const vk::SubpassDescription> subpasses,
      vk::ArrayProxy<const vk::SubpassDependency> dependencies );

    std::shared_ptr<Queue> getQueue( uint32_t familyIndex, uint32_t queueIndex );

    std::shared_ptr<PhysicalDevice> _physicalDevice;

    std::shared_ptr<Swapchain> createSwapchain( const std::shared_ptr<Surface>& surface,
      uint32_t numImageCount, vk::Format imageFormat, vk::ColorSpaceKHR colorSpace,
      const vk::Extent2D& imageExtent,  uint32_t imageArrayLayers, 
      vk::ImageUsageFlags imageUsage, vk::SharingMode imageSharingMode,
      const std::vector<uint32_t>& queueFamilyIndices,
      vk::SurfaceTransformFlagBitsKHR preTransform, vk::CompositeAlphaFlagBitsKHR compositeAlpha,
      vk::PresentModeKHR presentMode, bool clipped, const std::shared_ptr<Swapchain>& oldSwapchain );


    std::shared_ptr<Image> createImage( vk::ImageCreateFlags createFlags, vk::ImageType type, vk::Format format,
      const vk::Extent3D & extent, uint32_t mipLevels, uint32_t arraySize, vk::SampleCountFlagBits samples, vk::ImageTiling tiling,
      vk::ImageUsageFlags usageFlags, vk::SharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices,
      vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryPropertyFlags );

    std::shared_ptr<Framebuffer> createFramebuffer( const std::shared_ptr<RenderPass>& renderPass,
      const std::vector<std::shared_ptr<ImageView>>& attachments, const vk::Extent2D& extent, uint32_t layers );

    std::shared_ptr<CommandPool> createCommandPool( vk::CommandPoolCreateFlags flags = {}, uint32_t familyIndex = 0 );

    std::shared_ptr<ShaderModule> createShaderModule( const std::string& filePath );
    std::shared_ptr<ShaderModule> createShaderModule( vk::ArrayProxy<const uint32_t> code );

    std::shared_ptr<Fence> createFence( bool signaled );
    std::shared_ptr<Sampler> createSampler( const SamplerStateDesc & desc );
    std::shared_ptr<DescriptorSetLayout> createDescriptorSetLayout(
      vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings );
    std::shared_ptr<DescriptorPool> createDescriptorPool(
      vk::DescriptorPoolCreateFlags flags, uint32_t maxSets,
      vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes );


    /**
    * Allocates memory for the provided image, and binds it to the image.
    * Returns null if it cannot find memory with the specified flags.
    */
    vk::DeviceMemory allocateImageMemory( vk::Image image, vk::MemoryPropertyFlags flags );

    /**
    * Allocates memory for the provided buffer, and binds it to the buffer.
    * Returns null if it cannot find memory with the specified flags.
    */
    vk::DeviceMemory allocateBufferMemory( vk::Buffer buffer, vk::MemoryPropertyFlags flags );

    /**
    * Allocates a block of memory according to the provided memory requirements.
    * Returns null if it cannot find memory with the specified flags.
    */
    vk::DeviceMemory allocateMemReqMemory( const vk::MemoryRequirements& reqs,
      vk::MemoryPropertyFlags flags );

    // Frees a previously allocated block of memory.
    void freeMemory( vk::DeviceMemory memory );

  protected:
    void init(
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures );

    // Types of GPU queues.
    enum GpuQueueType : short
    {
      /**
      * Queue used for rendering. Allows the use of draw commands,
      * but also all commands supported by compute or upload buffers.
      */
      GPUT_GRAPHICS,
      /** Discrete queue used for compute operations. Allows the use of dispatch and upload commands. */
      GPUT_COMPUTE,
      /** Queue used for memory transfer operations only. No rendering or compute dispatch allowed. */
      GPUT_TRANSFER,
      GPUT_COUNT
    };
    /*// Contains data about a set of queues of a specific type.
    struct QueueInfo
    {
      uint32_t familyIdx = -1;
      std::vector<vk::Queue> queues;
    };
    std::array< QueueInfo, GPUT_COUNT> _queueInfos;*/

    vk::Device _device;

    std::map<uint32_t, std::vector<std::unique_ptr<Queue>>> _queues; // key is queueFamilyIndex
  };
}

#endif /* __LAVA_DEVICE__ */