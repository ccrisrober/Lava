

int main( void )
{
	std::shared_ptr< lava::Instance > instance;


	VulkanWindow w;
	w.setVulkanInstance( instance );
	w.show( );
}