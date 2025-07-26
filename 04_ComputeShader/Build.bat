cls

del Vk.exe

del Vk.obj

del Vk.res

del shader.vert.spv
del shader.frag.spv
del shader.comp.spv

cl.exe /c /EHsc /I "C:\VulkanSDK\Vulkan\Include" Vk.cpp

rc.exe Vk.rc

link.exe Vk.obj Vk.res /LIBPATH:"C:\VulkanSDK\Vulkan\Lib" user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

glslangValidator.exe -V -H -o shader.vert.spv shader.vert

glslangValidator.exe -V -H -o shader.frag.spv shader.frag

glslangValidator.exe -V -H -o shader.comp.spv shader.comp

