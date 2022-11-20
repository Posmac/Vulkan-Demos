@ECHO OFF
D:/VulkanSDK/1.3.216.0/Bin/glslc.exe vertexDiffuseLighting/shader.vert -o vertexDiffuseLighting/shaderSPIRV.vert.txt
D:/VulkanSDK/1.3.216.0/Bin/glslc.exe vertexDiffuseLighting/shader.frag -o vertexDiffuseLighting/shaderSPIRV.frag.txt

D:/VulkanSDK/1.3.216.0/Bin/glslc.exe pixelDiffuseLighting/shader.vert -o pixelDiffuseLighting/shaderSPIRV.vert.txt
D:/VulkanSDK/1.3.216.0/Bin/glslc.exe pixelDiffuseLighting/shader.frag -o pixelDiffuseLighting/shaderSPIRV.frag.txt