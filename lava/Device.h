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
#include "Buffer.h"
#include "Event.h"

#include <vector>
#include <map>

#include "noncopyable.hpp"

namespace lava
{
  uint32_t findMemoryType( const vk::PhysicalDeviceMemoryProperties& memProps, 
    uint32_t reqBits, vk::MemoryPropertyFlags wantedFlags );
  class PhysicalDevice;
  class Device : 
    private NonCopyable<Device>, 
    public std::enable_shared_from_this<Device>
  {
  public:
    LAVA_API
    static DeviceRef create( const std::shared_ptr<PhysicalDevice>& phyDev,
      const std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos,
      const std::vector<std::string>& enabledLayerNames,
      const std::vector<std::string>& enabledExtensionNames,
      const vk::PhysicalDeviceFeatures& enabledFeatures );
    LAVA_API
    Device( const std::shared_ptr<PhysicalDevice>& phyDev );
    LAVA_API
    virtual ~Device( void );
    LAVA_API
    inline operator vk::Device( void ) const
    {
      return _device;
    }
    LAVA_API
    vk::Result waitForFences( vk::ArrayProxy<const std::shared_ptr<Fence>> fences, 
      bool waitAll, uint32_t timeout ) const;

    LAVA_API
    std::shared_ptr<Buffer> createBuffer( vk::BufferCreateFlags createFlags, 
      vk::DeviceSize size, vk::BufferUsageFlags usageFlags = 
        vk::BufferUsageFlagBits::eTransferDst,
        vk::SharingMode sharingMode = vk::SharingMode::eExclusive, 
        vk::ArrayProxy<const uint32_t> queueFamilyIndices = nullptr,
        vk::MemoryPropertyFlags memoryPropertyFlags 
          = vk::MemoryPropertyFlagBits::eDeviceLocal );
    LAVA_API
    std::shared_ptr<Buffer> createBuffer( vk::DeviceSize size, 
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eTransferDst, 
      vk::SharingMode sharingMode = vk::SharingMode::eExclusive,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices = nullptr, 
      vk::MemoryPropertyFlags memoryPropertyFlags 
        = vk::MemoryPropertyFlagBits::eDeviceLocal );
    LAVA_API
    std::shared_ptr<Buffer> createBuffer( vk::DeviceSize size, 
      const BufferType& bufferType, 
      vk::SharingMode sharingMode = vk::SharingMode::eExclusive,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices = nullptr, 
      vk::MemoryPropertyFlags memoryPropertyFlags 
        = vk::MemoryPropertyFlagBits::eDeviceLocal );

    LAVA_API
    std::shared_ptr<DescriptorSet> allocateDescriptorSet( 
      const std::shared_ptr<DescriptorPool>& pool, 
      const std::shared_ptr<DescriptorSetLayout>& layout );
    LAVA_API
    void updateDescriptorSets( 
      std::vector<WriteDescriptorSet> descriptorWrites,
      std::vector<CopyDescriptorSet> descriptorCopies );

    LAVA_API
    std::shared_ptr<Event> createEvent( void );

    LAVA_API
    void waitIdle( void );

    LAVA_API
    std::shared_ptr<Semaphore> createSemaphore( void );

    LAVA_API
    std::shared_ptr<RenderPass> createRenderPass(
      vk::ArrayProxy<const vk::AttachmentDescription> attachments,
      vk::ArrayProxy<const vk::SubpassDescription> subpasses,
      vk::ArrayProxy<const vk::SubpassDependency> dependencies );

    LAVA_API
    std::shared_ptr<Queue> getQueue( uint32_t familyIndex, uint32_t queueIdx );

    LAVA_API
    std::shared_ptr<Swapchain> createSwapchain( const std::shared_ptr<Surface>& surface,
      uint32_t numImageCount, vk::Format imageFormat, vk::ColorSpaceKHR colorSpace,
      const vk::Extent2D& imageExtent,  uint32_t imageArrayLayers, 
      vk::ImageUsageFlags imageUsage, vk::SharingMode imageSharingMode,
      const std::vector<uint32_t>& queueFamilyIndices,
      vk::SurfaceTransformFlagBitsKHR preTransform, 
      vk::CompositeAlphaFlagBitsKHR compositeAlpha,
      vk::PresentModeKHR presentMode, bool clipped, 
      const std::shared_ptr<Swapchain>& oldSwapchain );

    LAVA_API
    std::shared_ptr<Image> createImage( vk::ImageCreateFlags createFlags, vk::ImageType type, vk::Format format,
      const vk::Extent3D & extent, uint32_t mipLevels, uint32_t arraySize, vk::SampleCountFlagBits samples, vk::ImageTiling tiling,
      vk::ImageUsageFlags usageFlags, vk::SharingMode sharingMode, const std::vector<uint32_t>& queueFamilyIndices,
      vk::ImageLayout initialLayout, vk::MemoryPropertyFlags memoryPropertyFlags );

    LAVA_API
    std::shared_ptr<Framebuffer> createFramebuffer( const std::shared_ptr<RenderPass>& renderPass,
      const std::vector<std::shared_ptr<ImageView>>& attachments, const vk::Extent2D& extent, uint32_t layers );

    LAVA_API
    std::shared_ptr<CommandPool> createCommandPool( vk::CommandPoolCreateFlags flags = {}, uint32_t familyIndex = 0 );

    LAVA_API
    std::shared_ptr<ShaderModule> createShaderModule( const std::string& filePath, vk::ShaderStageFlagBits type );
    LAVA_API
    std::shared_ptr<ShaderModule> createShaderModule( const std::string& filePath );
    LAVA_API
    std::shared_ptr<ShaderModule> createShaderModule( vk::ArrayProxy<const uint32_t> code );


    LAVA_API
    const PipelineShaderStageCreateInfo createShaderPipelineShaderStage(
      const std::string& spvFile, vk::ShaderStageFlagBits stage, 
      vk::Optional<const SpecializationInfo> specializationInfo = nullptr );

    LAVA_API
    std::shared_ptr<Fence> createFence( bool signaled );
    LAVA_API
    std::shared_ptr<Sampler> createSampler( const SamplerStateDesc & desc );
    LAVA_API
    std::shared_ptr<DescriptorSetLayout> createDescriptorSetLayout(
      vk::ArrayProxy<const DescriptorSetLayoutBinding> bindings, 
      vk::DescriptorSetLayoutCreateFlags flags = { } );
    LAVA_API
    std::shared_ptr<DescriptorPool> createDescriptorPool(
      /*vk::DescriptorPoolCreateFlags flags, */uint32_t maxSets,
      vk::ArrayProxy<const vk::DescriptorPoolSize> poolSizes );

    LAVA_API
    std::shared_ptr<PhysicalDevice> getPhysicalDevice( void ) const
    {
      return _physicalDevice;
    }

    LAVA_API
    std::shared_ptr<PipelineCache> createPipelineCache( size_t initialSize, 
      void const* initialData = nullptr );

    LAVA_API
    std::shared_ptr<Pipeline> createGraphicsPipeline( 
      const std::shared_ptr<PipelineCache>& pipelineCache, 
      vk::PipelineCreateFlags flags, 
      vk::ArrayProxy<const PipelineShaderStageCreateInfo> stages,
      vk::Optional<const PipelineVertexInputStateCreateInfo> vertexInputState,
      vk::Optional<const vk::PipelineInputAssemblyStateCreateInfo> inputAssemblyState,
      vk::Optional<const vk::PipelineTessellationStateCreateInfo> tessellationState,
      vk::Optional<const PipelineViewportStateCreateInfo> viewportState,
      vk::Optional<const vk::PipelineRasterizationStateCreateInfo> rasterizationState,
      vk::Optional<const PipelineMultisampleStateCreateInfo> multisampleState,
      vk::Optional<const vk::PipelineDepthStencilStateCreateInfo> depthStencilState,
      vk::Optional<const PipelineColorBlendStateCreateInfo> colorBlendState,
      vk::Optional<const PipelineDynamicStateCreateInfo> dynamicState,
      std::shared_ptr<PipelineLayout> const& pipelineLayout, 
      std::shared_ptr<RenderPass> const& renderPass, uint32_t subpass = 0,
      std::shared_ptr<Pipeline> const& basePipelineHandle = {}, 
      uint32_t basePipelineIndex = 0 );

    LAVA_API
    std::shared_ptr<Pipeline> createComputePipeline( 
      const std::shared_ptr<PipelineCache>& pipelineCache, 
      vk::PipelineCreateFlags flags,
      const PipelineShaderStageCreateInfo& stage,
      const std::shared_ptr<PipelineLayout>& pipelineLayout,
      const std::shared_ptr<Pipeline>& basePipelineHandle = {},
      uint32_t basePipelineIndex = 0 );

    LAVA_API
    std::shared_ptr<PipelineLayout> createPipelineLayout( 
      vk::ArrayProxy<const std::shared_ptr<DescriptorSetLayout>> setLayouts,
      vk::ArrayProxy<const vk::PushConstantRange> pushConstantRanges = nullptr );

    /**
    * Allocates memory for the provided image, and binds it to the image.
    * Returns null if it cannot find memory with the specified flags.
    */
    LAVA_API
    vk::DeviceMemory allocateImageMemory( vk::Image image, vk::MemoryPropertyFlags flags );

    /**
    * Allocates memory for the provided buffer, and binds it to the buffer.
    * Returns null if it cannot find memory with the specified flags.
    */
    LAVA_API
    vk::DeviceMemory allocateBufferMemory( vk::Buffer buffer, vk::MemoryPropertyFlags flags );

    /**
    * Allocates a block of memory according to the provided memory requirements.
    * Returns null if it cannot find memory with the specified flags.
    */
    LAVA_API
    vk::DeviceMemory allocateMemReqMemory( const vk::MemoryRequirements& reqs,
      vk::MemoryPropertyFlags flags );

    // Frees a previously allocated block of memory.
    LAVA_API
    void freeMemory( vk::DeviceMemory memory );

#ifdef LAVA_DEVICE_BUILDERS
    LAVA_API
    std::shared_ptr<UniformBuffer> createUniformBuffer( vk::DeviceSize size );
    LAVA_API
    std::shared_ptr<StorageBuffer> createStorageBuffer( vk::DeviceSize size );
    LAVA_API
    std::shared_ptr<UniformTexelBuffer> createUniformTexelBuffer( vk::DeviceSize size );
    LAVA_API
    std::shared_ptr<VertexBuffer> createVertexBuffer( vk::DeviceSize size );
    LAVA_API
    std::shared_ptr<IndexBuffer> createIndexBuffer( vk::IndexType type, 
      vk::DeviceSize size );
#endif

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
    std::shared_ptr<PhysicalDevice> _physicalDevice;

    std::map<uint32_t, std::vector<std::unique_ptr<Queue>>> _queues; // key is queueFamilyIndex
  };
}

#endif /* __LAVA_DEVICE__ */