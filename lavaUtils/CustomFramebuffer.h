#pragma once

#include <lava/lava.h>
#include <lavaUtils/api.h>

namespace lava
{
  namespace utility
  {
		struct FramebufferAttachment
		{
		  std::shared_ptr<Image> image;
		  std::shared_ptr<ImageView> imageView;
		  vk::ImageLayout finalLayout;
		  vk::ImageSubresourceRange subresourceRange;
		  vk::AttachmentDescription description;
		  vk::Format format;
		  
		  LAVAUTILS_API
			bool hasDepth( void )
		  {
			static std::vector<vk::Format> formats =
			{
			  vk::Format::eD16Unorm,
			  vk::Format::eX8D24UnormPack32,
			  vk::Format::eD32Sfloat,
			  vk::Format::eD16UnormS8Uint,
			  vk::Format::eD24UnormS8Uint,
			  vk::Format::eD32SfloatS8Uint
			};
			return std::find(
			  formats.begin( ), formats.end( ), format
			  ) != std::end( formats );
		  }
      LAVAUTILS_API
			bool hasStencil( void )
		  {
			  static std::vector<vk::Format> formats =
			  {
			    vk::Format::eS8Uint,
			    vk::Format::eD16UnormS8Uint,
			    vk::Format::eD24UnormS8Uint,
			    vk::Format::eD32SfloatS8Uint
			  };
			  return std::find(
			    formats.begin( ), formats.end( ), format
			  ) != std::end( formats );
		  }
      LAVAUTILS_API
			bool isDepthStencil( void )
		  {
			  return( hasDepth( ) || hasStencil( ) );
		  }
		};

		/*struct AttachmentInfo
		{
			uint32_t width, height;
			vk::Format format;
			vk::ImageUsageFlagBits usage;
		};*/
		class CustomFramebuffer : public VulkanResource
		{
		protected:
    public:
		  std::vector< FramebufferAttachment > attachments;
      std::shared_ptr< Sampler> _sampler;
      std::shared_ptr< RenderPass > _renderPass;
      std::shared_ptr< Framebuffer > _framebuffer;
      uint32_t _width, _height;
      std::shared_ptr<Semaphore> semaphore;
		public:
		  LAVAUTILS_API
		  CustomFramebuffer( std::shared_ptr< Device > device, uint32_t w, uint32_t h );

		  LAVAUTILS_API
		  virtual ~CustomFramebuffer( void );

		  LAVAUTILS_API
			uint32_t addAttachment( uint32_t width_, uint32_t height_, 
				vk::Format format_, vk::ImageUsageFlags usage_ );
      LAVAUTILS_API
      void createSampler( vk::Filter magFilter, vk::Filter minFilter, 
        vk::SamplerAddressMode addressMode );

      LAVAUTILS_API
      void createRenderPass( void );
		};
  }
}
