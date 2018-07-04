#include "Compiler.h"
#include <glslang/SPIRV/GlslangToSpv.h>

#include <iostream>

namespace lava
{
  namespace utility
  {
    class GLSLToSPIRVCompiler
    {
    public:
      GLSLToSPIRVCompiler( void );
      ~GLSLToSPIRVCompiler( void );

      LAVAUTILS_API
      std::vector<uint32_t> compile(vk::ShaderStageFlagBits stage, 
        const std::string& source) const;

    private:
      TBuiltInResource _resource;
    };

    GLSLToSPIRVCompiler::GLSLToSPIRVCompiler( void )
    {
  #ifndef __ANDROID__
      glslang::InitializeProcess( );
  #endif

      _resource.maxLights = 32;
      _resource.maxClipPlanes = 6;
      _resource.maxTextureUnits = 32;
      _resource.maxTextureCoords = 32;
      _resource.maxVertexAttribs = 64;
      _resource.maxVertexUniformComponents = 4096;
      _resource.maxVaryingFloats = 64;
      _resource.maxVertexTextureImageUnits = 32;
      _resource.maxCombinedTextureImageUnits = 80;
      _resource.maxTextureImageUnits = 32;
      _resource.maxFragmentUniformComponents = 4096;
      _resource.maxDrawBuffers = 32;
      _resource.maxVertexUniformVectors = 128;
      _resource.maxVaryingVectors = 8;
      _resource.maxFragmentUniformVectors = 16;
      _resource.maxVertexOutputVectors = 16;
      _resource.maxFragmentInputVectors = 15;
      _resource.minProgramTexelOffset = -8;
      _resource.maxProgramTexelOffset = 7;
      _resource.maxClipDistances = 8;
      _resource.maxComputeWorkGroupCountX = 65535;
      _resource.maxComputeWorkGroupCountY = 65535;
      _resource.maxComputeWorkGroupCountZ = 65535;
      _resource.maxComputeWorkGroupSizeX = 1024;
      _resource.maxComputeWorkGroupSizeY = 1024;
      _resource.maxComputeWorkGroupSizeZ = 64;
      _resource.maxComputeUniformComponents = 1024;
      _resource.maxComputeTextureImageUnits = 16;
      _resource.maxComputeImageUniforms = 8;
      _resource.maxComputeAtomicCounters = 8;
      _resource.maxComputeAtomicCounterBuffers = 1;
      _resource.maxVaryingComponents = 60;
      _resource.maxVertexOutputComponents = 64;
      _resource.maxGeometryInputComponents = 64;
      _resource.maxGeometryOutputComponents = 128;
      _resource.maxFragmentInputComponents = 128;
      _resource.maxImageUnits = 8;
      _resource.maxCombinedImageUnitsAndFragmentOutputs = 8;
      _resource.maxCombinedShaderOutputResources = 8;
      _resource.maxImageSamples = 0;
      _resource.maxVertexImageUniforms = 0;
      _resource.maxTessControlImageUniforms = 0;
      _resource.maxTessEvaluationImageUniforms = 0;
      _resource.maxGeometryImageUniforms = 0;
      _resource.maxFragmentImageUniforms = 8;
      _resource.maxCombinedImageUniforms = 8;
      _resource.maxGeometryTextureImageUnits = 16;
      _resource.maxGeometryOutputVertices = 256;
      _resource.maxGeometryTotalOutputComponents = 1024;
      _resource.maxGeometryUniformComponents = 1024;
      _resource.maxGeometryVaryingComponents = 64;
      _resource.maxTessControlInputComponents = 128;
      _resource.maxTessControlOutputComponents = 128;
      _resource.maxTessControlTextureImageUnits = 16;
      _resource.maxTessControlUniformComponents = 1024;
      _resource.maxTessControlTotalOutputComponents = 4096;
      _resource.maxTessEvaluationInputComponents = 128;
      _resource.maxTessEvaluationOutputComponents = 128;
      _resource.maxTessEvaluationTextureImageUnits = 16;
      _resource.maxTessEvaluationUniformComponents = 1024;
      _resource.maxTessPatchComponents = 120;
      _resource.maxPatchVertices = 32;
      _resource.maxTessGenLevel = 64;
      _resource.maxViewports = 16;
      _resource.maxVertexAtomicCounters = 0;
      _resource.maxTessControlAtomicCounters = 0;
      _resource.maxTessEvaluationAtomicCounters = 0;
      _resource.maxGeometryAtomicCounters = 0;
      _resource.maxFragmentAtomicCounters = 8;
      _resource.maxCombinedAtomicCounters = 8;
      _resource.maxAtomicCounterBindings = 1;
      _resource.maxVertexAtomicCounterBuffers = 0;
      _resource.maxTessControlAtomicCounterBuffers = 0;
      _resource.maxTessEvaluationAtomicCounterBuffers = 0;
      _resource.maxGeometryAtomicCounterBuffers = 0;
      _resource.maxFragmentAtomicCounterBuffers = 1;
      _resource.maxCombinedAtomicCounterBuffers = 1;
      _resource.maxAtomicCounterBufferSize = 16384;
      _resource.maxTransformFeedbackBuffers = 4;
      _resource.maxTransformFeedbackInterleavedComponents = 64;
      _resource.maxCullDistances = 8;
      _resource.maxCombinedClipAndCullDistances = 8;
      _resource.maxSamples = 4;
      _resource.limits.nonInductiveForLoops = 1;
      _resource.limits.whileLoops = 1;
      _resource.limits.doWhileLoops = 1;
      _resource.limits.generalUniformIndexing = 1;
      _resource.limits.generalAttributeMatrixVectorIndexing = 1;
      _resource.limits.generalVaryingIndexing = 1;
      _resource.limits.generalSamplerIndexing = 1;
      _resource.limits.generalVariableIndexing = 1;
      _resource.limits.generalConstantMatrixVectorIndexing = 1;
    }

    GLSLToSPIRVCompiler::~GLSLToSPIRVCompiler( void )
    {
  #ifndef __ANDROID__
      glslang::FinalizeProcess( );
  #endif
    }

    std::vector<uint32_t> GLSLToSPIRVCompiler::compile(
      vk::ShaderStageFlagBits stage, const std::string& source) const
    {
      static const std::map<vk::ShaderStageFlagBits, EShLanguage> stageToLanguageMap
      {
        {vk::ShaderStageFlagBits::eVertex, EShLangVertex},
        {vk::ShaderStageFlagBits::eTessellationControl, EShLangTessControl},
        {vk::ShaderStageFlagBits::eTessellationEvaluation, EShLangTessEvaluation},
        {vk::ShaderStageFlagBits::eGeometry, EShLangGeometry},
        {vk::ShaderStageFlagBits::eFragment, EShLangFragment},
        {vk::ShaderStageFlagBits::eCompute, EShLangCompute}
      };

      std::map<vk::ShaderStageFlagBits, EShLanguage>::const_iterator stageIt = 
        stageToLanguageMap.find(stage);
      assert( stageIt != stageToLanguageMap.end());
      glslang::TShader shader(stageIt->second);

      const char *shaderStrings[1];
      shaderStrings[0] = source.c_str();
      shader.setStrings(shaderStrings, 1);

      // Enable SPIR-V and Vulkan rules when parsing GLSL
      EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

      if (!shader.parse(&_resource, 100, false, messages))
      {
        std::string infoLog = shader.getInfoLog();
        std::string infoDebugLog = shader.getInfoDebugLog();
        assert(false);
      }

      glslang::TProgram program;
      program.addShader(&shader);

      if (!program.link(messages))
      {
        std::string infoLog = program.getInfoLog();
        std::string infoDebugLog = program.getInfoDebugLog();
        assert(false);
      }

      std::vector<uint32_t> code;
      glslang::GlslangToSpv(*program.getIntermediate(stageIt->second), code);

      return code;
    }

    std::vector<uint32_t> compileGLSLToSPIRV(vk::ShaderStageFlagBits stage, 
      const std::string& source)
    {
      static GLSLToSPIRVCompiler compiler;
      return compiler.compile(stage, source);
    }
  }
}