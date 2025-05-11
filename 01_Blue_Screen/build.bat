cls

del Source.exe

cl.exe /c /EHsc /std:c++17 Source.cpp  -I "C:\VulkanSDK\Vulkan\Include"

cl.exe /c /EHsc /std:c++17 vk_engine.cpp  -I "C:\VulkanSDK\Vulkan\Include"

link.exe Source.obj vk_engine.obj user32.lib gdi32.lib kernel32.lib "C:\VulkanSDK\Vulkan\Lib\vulkan-1.lib" /MACHINE:X64

del Source.obj vk_engine.obj

Source.exe
