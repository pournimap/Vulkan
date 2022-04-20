cls

del Source.exe

C:\VulkanSDK\1.3.204.1\Bin\glslc.exe shaders\shader.vert -o shaders\vert.spv
C:\VulkanSDK\1.3.204.1\Bin\glslc.exe shaders\shader.frag -o shaders\frag.spv

cl.exe /c /EHsc /std:c++17 Source.cpp  -I "C:\VulkanSDK\1.3.204.1\Include"

cl.exe /c /EHsc /std:c++17 vk_engine.cpp  -I "C:\VulkanSDK\1.3.204.1\Include"

link.exe Source.obj vk_engine.obj user32.lib gdi32.lib kernel32.lib "C:\VulkanSDK\1.3.204.1\Lib\vulkan-1.lib" /MACHINE:X64

del Source.obj vk_engine.obj

Source.exe
