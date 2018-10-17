#include "Compiler.h"

#include <iostream>

namespace pompeii
{
  namespace utility
  {
    EShLanguage FindLanguage(const vk::ShaderStageFlagBits shaderType)
    {
      switch (shaderType) {
        case vk::ShaderStageFlagBits::eVertex:
          return EShLangVertex;

        case vk::ShaderStageFlagBits::eTessellationControl:
          return EShLangTessControl;

        case vk::ShaderStageFlagBits::eTessellationEvaluation:
          return EShLangTessEvaluation;

        case vk::ShaderStageFlagBits::eGeometry:
          return EShLangGeometry;

        case vk::ShaderStageFlagBits::eFragment:
          return EShLangFragment;

        case vk::ShaderStageFlagBits::eCompute:
          return EShLangCompute;

        default:
          return EShLangVertex;
      }
    }

    void init_resources(TBuiltInResource &_resource)
    {
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

    bool GLSLtoSPV(vk::ShaderStageFlagBits shaderType, const char *pshader, 
      std::vector<uint32_t> &spirv)
    {
      glslang::InitializeProcess();

      EShLanguage stage = FindLanguage(shaderType);
      glslang::TShader shader(stage);
      glslang::TProgram program;

      const char *shaderStrings[1];

      TBuiltInResource Resources;
      init_resources(Resources);

      // Enable SPIR-V and Vulkan rules when parsing GLSL
      EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

      shaderStrings[0] = pshader;

      shader.setStrings(shaderStrings, 1);

      std::cout << "Parsing " << std::endl;
      if (!shader.parse(&Resources, 100, false, messages))
      {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return false;
      }
      std::cout << "Parsing OK" << std::endl;

      program.addShader(&shader);
      
      // Link the program and report if errors...
      std::cout << "Linking " << std::endl;
      if (!program.link(messages)) {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return false;
      }
      std::cout << "Linking OK" << std::endl;

      glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

      glslang::FinalizeProcess();
      
      return true;
    }
  }
}