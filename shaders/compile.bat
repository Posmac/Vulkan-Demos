@ECHO OFF
D:/VulkanSDK/1.3.216.0/Bin/glslc.exe VertexDiffuseLighting/shader.vert -o VertexDiffuseLighting/shaderSPIRV.vert.txt
D:/VulkanSDK/1.3.216.0/Bin/glslc.exe VertexDiffuseLighting/shader.frag -o VertexDiffuseLighting/shaderSPIRV.frag.txt

D:/VulkanSDK/1.3.216.0/Bin/glslc.exe PixelDiffuseLighting/shader.vert -o PixelDiffuseLighting/shaderSPIRV.vert.txt
D:/VulkanSDK/1.3.216.0/Bin/glslc.exe PixelDiffuseLighting/shader.frag -o PixelDiffuseLighting/shaderSPIRV.frag.txt

D:/VulkanSDK/1.3.216.0/Bin/glslc.exe BumpMapping/shader.vert -o BumpMapping/shaderSPIRV.vert.txt
D:/VulkanSDK/1.3.216.0/Bin/glslc.exe BumpMapping/shader.frag -o BumpMapping/shaderSPIRV.frag.txt