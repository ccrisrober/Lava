#ifndef __POMPEIIUTILS_WRAPPERS__
#define __POMPEIIUTILS_WRAPPERS__

#include <pompeii/pompeii.h>
#include <pompeiiUtils/api.h>

namespace pompeii
{
  namespace utils
  {
    struct FramebufferColorAttachment
    {
      std::shared_ptr<Image> image;
      std::shared_ptr<ImageView> imageView;
    };
    class OffscreenFBO: public VulkanResource
    {
    public:
      OffscreenFBO( const std::shared_ptr<Device>& device,
        uint32_t w, uint32_t h );
      void resize( uint32_t w, uint32_t h );
    protected:
      uint32_t _width, _height;
      std::shared_ptr<RenderPass> _renderPass;
      std::shared_ptr<Framebuffer> _framebuffer;
      FramebufferColorAttachment colorAttachment;
      FramebufferColorAttachment depthBuffer;
    };
    class RenderPassBuilder
    {
    public:
      POMPEIIUTILS_API
      std::shared_ptr<pompeii::RenderPass> createRenderPass( 
        const std::shared_ptr<Device>& device )
      {
        return std::make_shared< pompeii::RenderPass >( device, 
          attachmentDescriptions, subpassDescriptions, subpassDependendies );
      }
      POMPEIIUTILS_API
      void setAttachment( vk::Format format, vk::SampleCountFlagBits samples,
        vk::ImageLayout initialLayout, vk::ImageLayout finalLayout,
        vk::AttachmentLoadOp loadOp )
      {
        vk::AttachmentDescription attachment;
        attachment
          .setFormat( format )
          .setSamples( samples )
          .setLoadOp( loadOp )
          .setStoreOp( vk::AttachmentStoreOp::eStore )
          .setStencilLoadOp( vk::AttachmentLoadOp::eDontCare )
          .setStencilStoreOp( vk::AttachmentStoreOp::eDontCare )
          .setInitialLayout( initialLayout )
          .setFinalLayout( finalLayout );
        attachmentDescriptions.push_back( attachment );
      }

      POMPEIIUTILS_API
      void setSubpassDependency( uint32_t srcSubpass, uint32_t dstSubpass,
        vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, 
        vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
        vk::DependencyFlagBits dependencyFlags )
      {
        vk::SubpassDependency dependencies;
        dependencies
          .setSrcSubpass( srcSubpass )
          .setDstSubpass( dstSubpass )
          .setSrcStageMask( srcStageMask )
          .setDstStageMask( dstStageMask )
          .setSrcAccessMask( srcAccessMask )
          .setDstAccessMask( dstAccessMask )
          .setDependencyFlags( dependencyFlags );
        subpassDependendies.push_back( dependencies );
      }

      POMPEIIUTILS_API
      void addColorAttachmentReference( int location, vk::ImageLayout layout )
      {
        colorReferences.push_back( vk::AttachmentReference( location, layout ) );
      }

      POMPEIIUTILS_API
      void addDepthAttachmentReference( int location, vk::ImageLayout layout )
      {
        depthReference = vk::AttachmentReference( location, layout );
      }

      POMPEIIUTILS_API
      void createSubpass( void )
      {
        vk::SubpassDescription subpass;
        subpass
          .setPipelineBindPoint( vk::PipelineBindPoint::eGraphics )
          .setPInputAttachments( nullptr )
          .setColorAttachmentCount( colorReferences.size( ) )
          .setPColorAttachments( colorReferences.data( ) )
          .setPResolveAttachments( nullptr )
          .setPDepthStencilAttachment( &depthReference )
          .setPPreserveAttachments( nullptr );

        colorReferences.clear( );

        subpassDescriptions.push_back( subpass );
      }

    protected:
      std::vector<vk::AttachmentDescription> attachmentDescriptions;
      std::vector<vk::SubpassDependency> subpassDependendies;
      std::vector<vk::SubpassDescription> subpassDescriptions;
      std::vector<vk::AttachmentReference> colorReferences;
      vk::AttachmentReference depthReference;
    };

    class ImageCopyCmdBuffer : public pompeii::CommandBuffer
    {
    public:
      POMPEIIUTILS_API
      ImageCopyCmdBuffer( std::shared_ptr<CommandPool> cmdPool );
      POMPEIIUTILS_API
      void record( const std::shared_ptr<Buffer>& stagingBuffer, 
        const std::shared_ptr<Image>& image, uint32_t width, uint32_t height );
    };

    class ComputeCmdBuffer : public pompeii::CommandBuffer
    {
    public:
      POMPEIIUTILS_API
      ComputeCmdBuffer( std::shared_ptr<CommandPool> cmdPool,
        const std::shared_ptr<ComputePipeline> pipeline,
        const std::shared_ptr<PipelineLayout> pipelineLayout,
        vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> dSets,
        int groupCountX, int groupCountY, int groupCountZ, 
        vk::CommandBufferUsageFlags usage = 
          vk::CommandBufferUsageFlagBits::eRenderPassContinue, 
        void* pushData = nullptr );
    protected:
      void record( std::shared_ptr<ComputePipeline> pipeline,
        const std::shared_ptr<PipelineLayout> pipelineLayout,
        vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> dSets,
        int groupCountX, int groupCountY, int groupCountZ, 
        vk::CommandBufferUsageFlags usage, void* pushData );
    };

    class MipMapGenerationCmdBuffer : public pompeii::CommandBuffer
    {
    public:
      POMPEIIUTILS_API
      MipMapGenerationCmdBuffer( std::shared_ptr<CommandPool> cmdPool,
        std::shared_ptr<Image> image, uint32_t width, uint32_t height, uint32_t mipLevels,
        vk::ImageLayout initialLayout, vk::AccessFlags initialSrcAccessMask,
        vk::PipelineStageFlags initialSrcStageMask,
        vk::ImageLayout finalLayout, vk::AccessFlags finalDstAccessMask,
        vk::PipelineStageFlags finalDstStageMask );
    };

    enum class VertexLayout
    {
      POS_NORMAL_UV_TAN_BITAN,
      POS_NORMAL,
      POS_UV,
      POS,
      POS_NORMAL_UV,
      POS2D,
      POS2D_UV
    };

    class VertexInput
    {
    public:
      vk::VertexInputBindingDescription bindingDescription;
      std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
      int binding;

      POMPEIIUTILS_API
      VertexInput( VertexLayout layout );

      POMPEIIUTILS_API
      PipelineVertexInputStateCreateInfo getPipelineVertexInput( void ) const;
    protected:
      void createBindingDescription( int binding, int attributeCount, int stride );
      void addVertexAttributeDescription( int location, vk::Format format, int offset );
    };

  }
}

#endif /* __POMPEIIUTILS_WRAPPERS__ */