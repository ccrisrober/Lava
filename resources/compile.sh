#!/bin/bash

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquad.vert -o fullquad_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquad.frag -o fullquad_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquadUV.vert -o fullquadUV_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquadUV.frag -o fullquadUV_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V triangle.vert -o triangle_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V triangle.frag -o triangle_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V cube.vert -o cube_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V cube.frag -o cube_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V compute_example.comp -o compute_example.spv
