Only for glm we write it as vk.c to vk.cpp

glslangValidator.exe -V -H -o shader.vert.spv shader.vert

glslangValidator.exe -V -H -o shader.frag.spv shader.frag


MPD_Validation : debugReportCallback() : Validation (0) = vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState->pVertexAttributeDescriptions does not have a Location 1 but vertex shader has an input variable at that Location. (This can be valid if vertexAttributeRobustness feature is enabled).
The Vulkan spec states: If vertexAttributeRobustness is not enabled and the pipeline is being created with vertex input state and pVertexInputState is not dynamic, then all variables with the Input storage class decorated with Location in the Vertex Execution Model OpEntryPoint must contain a location in VkVertexInputAttributeDescription::location (https://vulkan.lunarg.com/doc/view/1.4.313.2/windows/antora/spec/latest/chapters/pipelines.html#VUID-VkGraphicsPipelineCreateInfo-Input-07904)

