
// Push Constants
VkPushConstantRange lRange = {};
lRange.stageFlags          = _stage;
lRange.offset              = 0;
lRange.size                = 0;

vPushConstantDescs.reserve(_info.pushConstants.size());
for (auto const &i : _info.pushConstants) {
  uint32_t lTempSize;
  VkFormat lTempF;

  if (!getGLSLTypeInfo(i.type, lTempSize, lTempF)) {
    wLOG("Unknown uniform type '", i.type, "' -- ignore");
    continue;
  }

  vPushConstantDescs.emplace_back();
  auto *lAlias = &vPushConstantDescs.back();

  lAlias->stage       = _stage;
  lAlias->name        = i.name;
  lAlias->type        = i.type;
  lAlias->offset      = lRange.size;
  lAlias->size        = lTempSize;
  lAlias->guessedRole = guessRole(i.type, i.name);

  lRange.size += lTempSize;
}
  
layout(push_constant) uniform pushConstants {
  float test1;
} u_pushConstants;
And a fragment shader with another push-constant block with a different float value:

layout(push_constant) uniform pushConstants {
  layout(offset = 4) float test2;
} u_pushConstants;

std::array<vk::PushConstantRange,2> ranges = {
	vk::PushConstantRange{
	  vk::ShaderStageFlagBits::eVertex,
	  0,
	  sizeof(float)
	},
	vk::PushConstantRange{
	  vk::ShaderStageFlagBits::eFragment,
	  sizeof(float), // Push-constant range offset (Start after vertex push constants)
	  sizeof(float)
	}
};
std::array<float,1> constants = {123.f};
commandBufferDraw.pushConstants(
	pipelineLayout,
	vk::ShaderStageFlagBits::eVertex,
	0,
	sizeof(float),
	constants.data()
);
std::array<float,1> constants = {456.f};
commandBufferDraw.pushConstants(
	pipelineLayout,
	vk::ShaderStageFlagBits::eFragment,
	sizeof(float), // Offset in bytes
	sizeof(float),
	constants.data()
);


- (Updates will be flushed manually, the VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag will isn´t used)
- secondaryCDM failing in asserts calls
- VulkanUIOverlay (ImGUI)
- Check multiple push constants


GLFWVulkanWindow
	GLFWPOMPEII_API
	std::shared_ptr<Image> currentImage( void ) const
	{
	auto images = _swapchain->images( );
	auto idx = imageIdx - 1;
	auto count = _swapchain->count( );
	if(idx < 0) idx += count;
	idx %= count;
	return images.at( idx );
	}

	pompeii::utils::saveScreenshot( _window->device( ), "pepito.ppm", 
	size.width, size.height, _window->colorFormat( ), 
	_window->currentImage( ), _window->gfxCommandPool( ), _window->gfxQueue( ) );

Other
	- Add NonCopyable to Swapchain and others classes
	- Added spirv_cross
	- Actualizar el ImageIdx al llamar a endRender( ) o algo así


Material
	class Material
	{
		DescriptorSet_set;
	}

	static GraphicPipeline materialCompiler( Material* material, RenderPass renderPass )
	{
		// yave-master/ShaderModuleBase
		VertexShader ...
		FragmentShader ...


		return GraphicPipeline( ..., renderPass, ... )
	}


	class MaterialData
	{
		Referencia a cada uno de los shaders
		Depth, cull y blend enabled
		Tipo de primitiva
		Bindings
		Depth compare op
		Variables del tesselation stage
	}

	En un material, tanto DescriptorLayout como el PipelineLayout son estáticos
	porque son variables que no hace falta replicar.


namespace pompeii
{
	typedef std::lock_guard<std::mutex> MutexGuard;
	typedef std::unique_lock<std::mutex> MutexUniqueLock;

	template<typename T>
	using SharedPtr = std::shared_ptr<T>;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	typedef std::chrono::duration<int, std::milli> Millisecs;
}