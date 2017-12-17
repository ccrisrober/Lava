#include "Event.h"
#include "Device.h"

namespace lava
{
	Event::Event( const std::shared_ptr<Device>& device )
    : VulkanResource( device )
	{
		_event = static_cast< vk::Device >( *_device ).createEvent( vk::EventCreateInfo( ) );
	}

	Event::~Event( void )
	{
		static_cast< vk::Device >( *_device ).destroyEvent( _event );
	}

	bool Event::isSignaled( void ) const
	{
		vk::Result result = static_cast< vk::Device >( *_device ).getEventStatus( _event );
		assert( ( result == vk::Result::eEventSet ) || ( result == vk::Result::eEventReset ) );
		return( result == vk::Result::eEventSet );
	}

	void Event::reset( void )
	{
		static_cast< vk::Device >( *_device ).resetEvent( _event );
	}

	void Event::set( void )
	{
		static_cast< vk::Device >( *_device ).setEvent( _event );
	}
}