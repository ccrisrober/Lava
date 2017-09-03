#ifndef __LAVA_BUFFER__
#define __LAVA_BUFFER__

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

#include "CommandBuffer.h"

namespace lava
{
  class Buffer : public VulkanResource, private NonCopyable<Buffer>
  {
  public:
    LAVA_API
    Buffer( const DeviceRef& device, vk::BufferCreateFlags createFlags, 
      vk::DeviceSize size, vk::BufferUsageFlags usageFlags, 
      vk::SharingMode sharingMode,
      vk::ArrayProxy<const uint32_t> queueFamilyIndices, 
      vk::MemoryPropertyFlags memoryPropertyFlags );
    LAVA_API
    virtual ~Buffer( );

    LAVA_API
    void* map( vk::DeviceSize offset, vk::DeviceSize length ) const;
    LAVA_API
    void unmap( void );
    template <typename T> void update( vk::DeviceSize offset, 
      vk::ArrayProxy<const T> data, std::shared_ptr<CommandBuffer> const& cmdBuff );

    inline operator vk::Buffer( ) const
    {
      return _buffer;
    }

  protected:
    vk::Buffer _buffer;
    vk::BufferView _view;
    vk::DeviceMemory _memory;
  };
  template<typename T>
  inline void Buffer::update( vk::DeviceSize offset, vk::ArrayProxy<const T> data, 
    std::shared_ptr<CommandBuffer> const & cmdBuff )
  {
    // todo
  }
}

#endif /* __LAVA_BUFFER__ */