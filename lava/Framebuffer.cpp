#include "Framebuffer.h"

#include "Device.h"
#include "RenderPass.h"

namespace lava
{
  Framebuffer::Framebuffer( const DeviceRef& device, const std::shared_ptr<RenderPass>& renderPass,
    const std::vector<std::shared_ptr<ImageView>>& attachments, const vk::Extent2D & extent, uint32_t layers )
    : VulkanResource( device )
    , _renderPass( renderPass )
    , _attachments( attachments.begin( ), attachments.end( ) )
  {
    std::vector<vk::ImageView> nativeAttachments;
    for ( auto const& att : _attachments )
    {
      nativeAttachments.push_back( *att );
    }

    vk::FramebufferCreateInfo fci;
    fci.setRenderPass( *_renderPass );
    fci.setAttachmentCount( nativeAttachments.size( ) );
    fci.setPAttachments( nativeAttachments.data( ) );
    fci.setWidth( extent.width );
    fci.setHeight( extent.height );
    fci.setLayers( layers );

    _framebuffer = static_cast< vk::Device >( *_device ).createFramebuffer( fci );
  }

  Framebuffer::~Framebuffer( void )
  {
    static_cast< vk::Device >( *_device ).destroyFramebuffer( _framebuffer );
  }
}