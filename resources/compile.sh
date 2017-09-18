#!/bin/bash

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquad.vert -o fullquad_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquad.frag -o fullquad_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquadUV.vert -o fullquadUV_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V fullquadUV.frag -o fullquadUV_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V triangle.vert -o triangle_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V triangle.frag -o triangle_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V cube.vert -o cube_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V cube.frag -o cube_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V cubeUV.vert -o cubeUV_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V cubeUV.frag -o cubeUV_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V mesh.vert -o mesh_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V mesh.frag -o mesh_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V compute_example.comp -o compute_example.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V parallax.vert -o normal_mapping_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V parallax.frag -o normal_mapping_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V instancing.vert -o instancing_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V instancing.frag -o instancing_frag.spv



/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V skybox.vert -o skybox_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V skybox.frag -o skybox_frag.spv

/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V reflect.vert -o reflect_vert.spv
/home/crodriguez/Vulkan/VulkanSDK/1.0.57.0/x86_64/bin/glslangValidator -V reflect.frag -o reflect_frag.spv
