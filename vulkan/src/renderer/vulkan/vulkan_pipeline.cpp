#include "vulkan_pipeline.h"
#include "vulkan_api.h"
#include "vulkan_type_converter.h"
#include "vulkan_renderpass.h"
#include "common/spirv_reflect.h"

#include <algorithm>

static uint32_t FormatSize(VkFormat format)
{
	uint32_t result = 0;
	switch (format) {
	case VK_FORMAT_UNDEFINED: result = 0; break;
	case VK_FORMAT_R4G4_UNORM_PACK8: result = 1; break;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16: result = 2; break;
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16: result = 2; break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16: result = 2; break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16: result = 2; break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16: result = 2; break;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16: result = 2; break;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16: result = 2; break;
	case VK_FORMAT_R8_UNORM: result = 1; break;
	case VK_FORMAT_R8_SNORM: result = 1; break;
	case VK_FORMAT_R8_USCALED: result = 1; break;
	case VK_FORMAT_R8_SSCALED: result = 1; break;
	case VK_FORMAT_R8_UINT: result = 1; break;
	case VK_FORMAT_R8_SINT: result = 1; break;
	case VK_FORMAT_R8_SRGB: result = 1; break;
	case VK_FORMAT_R8G8_UNORM: result = 2; break;
	case VK_FORMAT_R8G8_SNORM: result = 2; break;
	case VK_FORMAT_R8G8_USCALED: result = 2; break;
	case VK_FORMAT_R8G8_SSCALED: result = 2; break;
	case VK_FORMAT_R8G8_UINT: result = 2; break;
	case VK_FORMAT_R8G8_SINT: result = 2; break;
	case VK_FORMAT_R8G8_SRGB: result = 2; break;
	case VK_FORMAT_R8G8B8_UNORM: result = 3; break;
	case VK_FORMAT_R8G8B8_SNORM: result = 3; break;
	case VK_FORMAT_R8G8B8_USCALED: result = 3; break;
	case VK_FORMAT_R8G8B8_SSCALED: result = 3; break;
	case VK_FORMAT_R8G8B8_UINT: result = 3; break;
	case VK_FORMAT_R8G8B8_SINT: result = 3; break;
	case VK_FORMAT_R8G8B8_SRGB: result = 3; break;
	case VK_FORMAT_B8G8R8_UNORM: result = 3; break;
	case VK_FORMAT_B8G8R8_SNORM: result = 3; break;
	case VK_FORMAT_B8G8R8_USCALED: result = 3; break;
	case VK_FORMAT_B8G8R8_SSCALED: result = 3; break;
	case VK_FORMAT_B8G8R8_UINT: result = 3; break;
	case VK_FORMAT_B8G8R8_SINT: result = 3; break;
	case VK_FORMAT_B8G8R8_SRGB: result = 3; break;
	case VK_FORMAT_R8G8B8A8_UNORM: result = 4; break;
	case VK_FORMAT_R8G8B8A8_SNORM: result = 4; break;
	case VK_FORMAT_R8G8B8A8_USCALED: result = 4; break;
	case VK_FORMAT_R8G8B8A8_SSCALED: result = 4; break;
	case VK_FORMAT_R8G8B8A8_UINT: result = 4; break;
	case VK_FORMAT_R8G8B8A8_SINT: result = 4; break;
	case VK_FORMAT_R8G8B8A8_SRGB: result = 4; break;
	case VK_FORMAT_B8G8R8A8_UNORM: result = 4; break;
	case VK_FORMAT_B8G8R8A8_SNORM: result = 4; break;
	case VK_FORMAT_B8G8R8A8_USCALED: result = 4; break;
	case VK_FORMAT_B8G8R8A8_SSCALED: result = 4; break;
	case VK_FORMAT_B8G8R8A8_UINT: result = 4; break;
	case VK_FORMAT_B8G8R8A8_SINT: result = 4; break;
	case VK_FORMAT_B8G8R8A8_SRGB: result = 4; break;
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32: result = 4; break;
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32: result = 4; break;
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32: result = 4; break;
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32: result = 4; break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32: result = 4; break;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32: result = 4; break;
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32: result = 4; break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32: result = 4; break;
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32: result = 4; break;
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32: result = 4; break;
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32: result = 4; break;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32: result = 4; break;
	case VK_FORMAT_A2R10G10B10_SINT_PACK32: result = 4; break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32: result = 4; break;
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32: result = 4; break;
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32: result = 4; break;
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32: result = 4; break;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32: result = 4; break;
	case VK_FORMAT_A2B10G10R10_SINT_PACK32: result = 4; break;
	case VK_FORMAT_R16_UNORM: result = 2; break;
	case VK_FORMAT_R16_SNORM: result = 2; break;
	case VK_FORMAT_R16_USCALED: result = 2; break;
	case VK_FORMAT_R16_SSCALED: result = 2; break;
	case VK_FORMAT_R16_UINT: result = 2; break;
	case VK_FORMAT_R16_SINT: result = 2; break;
	case VK_FORMAT_R16_SFLOAT: result = 2; break;
	case VK_FORMAT_R16G16_UNORM: result = 4; break;
	case VK_FORMAT_R16G16_SNORM: result = 4; break;
	case VK_FORMAT_R16G16_USCALED: result = 4; break;
	case VK_FORMAT_R16G16_SSCALED: result = 4; break;
	case VK_FORMAT_R16G16_UINT: result = 4; break;
	case VK_FORMAT_R16G16_SINT: result = 4; break;
	case VK_FORMAT_R16G16_SFLOAT: result = 4; break;
	case VK_FORMAT_R16G16B16_UNORM: result = 6; break;
	case VK_FORMAT_R16G16B16_SNORM: result = 6; break;
	case VK_FORMAT_R16G16B16_USCALED: result = 6; break;
	case VK_FORMAT_R16G16B16_SSCALED: result = 6; break;
	case VK_FORMAT_R16G16B16_UINT: result = 6; break;
	case VK_FORMAT_R16G16B16_SINT: result = 6; break;
	case VK_FORMAT_R16G16B16_SFLOAT: result = 6; break;
	case VK_FORMAT_R16G16B16A16_UNORM: result = 8; break;
	case VK_FORMAT_R16G16B16A16_SNORM: result = 8; break;
	case VK_FORMAT_R16G16B16A16_USCALED: result = 8; break;
	case VK_FORMAT_R16G16B16A16_SSCALED: result = 8; break;
	case VK_FORMAT_R16G16B16A16_UINT: result = 8; break;
	case VK_FORMAT_R16G16B16A16_SINT: result = 8; break;
	case VK_FORMAT_R16G16B16A16_SFLOAT: result = 8; break;
	case VK_FORMAT_R32_UINT: result = 4; break;
	case VK_FORMAT_R32_SINT: result = 4; break;
	case VK_FORMAT_R32_SFLOAT: result = 4; break;
	case VK_FORMAT_R32G32_UINT: result = 8; break;
	case VK_FORMAT_R32G32_SINT: result = 8; break;
	case VK_FORMAT_R32G32_SFLOAT: result = 8; break;
	case VK_FORMAT_R32G32B32_UINT: result = 12; break;
	case VK_FORMAT_R32G32B32_SINT: result = 12; break;
	case VK_FORMAT_R32G32B32_SFLOAT: result = 12; break;
	case VK_FORMAT_R32G32B32A32_UINT: result = 16; break;
	case VK_FORMAT_R32G32B32A32_SINT: result = 16; break;
	case VK_FORMAT_R32G32B32A32_SFLOAT: result = 16; break;
	case VK_FORMAT_R64_UINT: result = 8; break;
	case VK_FORMAT_R64_SINT: result = 8; break;
	case VK_FORMAT_R64_SFLOAT: result = 8; break;
	case VK_FORMAT_R64G64_UINT: result = 16; break;
	case VK_FORMAT_R64G64_SINT: result = 16; break;
	case VK_FORMAT_R64G64_SFLOAT: result = 16; break;
	case VK_FORMAT_R64G64B64_UINT: result = 24; break;
	case VK_FORMAT_R64G64B64_SINT: result = 24; break;
	case VK_FORMAT_R64G64B64_SFLOAT: result = 24; break;
	case VK_FORMAT_R64G64B64A64_UINT: result = 32; break;
	case VK_FORMAT_R64G64B64A64_SINT: result = 32; break;
	case VK_FORMAT_R64G64B64A64_SFLOAT: result = 32; break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32: result = 4; break;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32: result = 4; break;

	default:
		break;
	}
	return result;
}

struct Shader
{
	VkShaderModule module;
	VkShaderStageFlagBits shaderStage;
	VkVertexInputBindingDescription bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
};

static VkShaderStageFlagBits get_shader_stage(SpvExecutionModel executionModel)
{
	switch (executionModel)
	{
	case SpvExecutionModelVertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case SpvExecutionModelFragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case SpvExecutionModelGLCompute:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	default:
		ASSERT_MSG(0, "Undefined Shader Stage");
	}
	return VkShaderStageFlagBits(0);
}

static void create_shader(Shader& shader, VkDevice device, const char* code, uint32_t size, std::vector<VkDescriptorSetLayoutBinding>& setLayouts, std::vector<VkPushConstantRange>& pushConstatRanges)
{
	VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = size;
	uint32_t* data = reinterpret_cast<uint32_t*>(const_cast<char*>(code));
	createInfo.pCode = data;
	VK_CHECK(vkCreateShaderModule(device, &createInfo, nullptr, &shader.module));

	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(size, code, &module);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	shader.shaderStage = get_shader_stage(module.spirv_execution_model);
	uint32_t attributeCount = 0;
	result = spvReflectEnumerateInputVariables(&module, &attributeCount, nullptr);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	// Bindings
	VkVertexInputBindingDescription bindingDesc = {};
	bindingDesc.binding = 0;
	bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	// computed later
	bindingDesc.stride = 0;

	// Input Attributes
	std::vector<SpvReflectInterfaceVariable*> reflInputAttributes(attributeCount);
	result = spvReflectEnumerateInputVariables(&module, &attributeCount, reflInputAttributes.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	if (shader.shaderStage != VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		for (uint32_t i = 0; i < attributeCount; ++i)
		{
			auto& reflData = reflInputAttributes[i];
			if (reflData->location > 32)
				continue;
			VkVertexInputAttributeDescription attributeDescription;
			attributeDescription.binding = bindingDesc.binding;
			attributeDescription.location = reflData->location;
			attributeDescription.format = static_cast<VkFormat>(reflData->format);
			attributeDescription.offset = 0;
			attributeDescriptions.push_back(attributeDescription);
		}

		std::sort(attributeDescriptions.begin(), attributeDescriptions.end(), [](const VkVertexInputAttributeDescription& a, const VkVertexInputAttributeDescription& b) {
			return a.location < b.location; });

		for (auto& attribute : attributeDescriptions)
		{
			uint32_t format_size = FormatSize(attribute.format);
			attribute.offset = bindingDesc.stride;
			bindingDesc.stride += format_size;
		}
	}

	shader.bindingDescriptions = bindingDesc;
	shader.attributeDescriptions = attributeDescriptions;

	// DescriptorSet Layouts
	uint32_t descriptorSetCounts = 0;
	result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCounts, nullptr);
	assert(result == SPV_REFLECT_RESULT_SUCCESS);
	std::vector<SpvReflectDescriptorSet*> reflDescriptorSets(descriptorSetCounts);
	result = spvReflectEnumerateDescriptorSets(&module, &descriptorSetCounts, reflDescriptorSets.data());
	assert(result == SPV_REFLECT_RESULT_SUCCESS);

	for (const auto& reflDescSet : reflDescriptorSets)
	{
		for (uint32_t i = 0; i < reflDescSet->binding_count; ++i)
		{
			const auto& reflBinding = reflDescSet->bindings[i];
			VkDescriptorSetLayoutBinding layoutBinding = {};
			layoutBinding.binding = reflBinding->binding;
			layoutBinding.descriptorType = static_cast<VkDescriptorType>(reflBinding->descriptor_type);
			layoutBinding.descriptorCount = 1;
			layoutBinding.stageFlags = shader.shaderStage;

			setLayouts.push_back(layoutBinding);
		}
	}

	// Push Constants
	uint32_t pushConstantCount = 0;
	result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, nullptr);
	ASSERT(result == SPV_REFLECT_RESULT_SUCCESS)
	std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
	result = spvReflectEnumeratePushConstantBlocks(&module, &pushConstantCount, pushConstants.data());
	ASSERT(result == SPV_REFLECT_RESULT_SUCCESS);

	for (auto& pushConstant : pushConstants)
	{
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.size = pushConstant->size;
		pushConstantRange.offset = pushConstant->offset;
		pushConstantRange.stageFlags = shader.shaderStage;
		pushConstatRanges.push_back(pushConstantRange);
	}
	spvReflectDestroyShaderModule(&module);
}

VkPipeline VulkanPipeline::create_graphics_pipeline(VkDevice device, VkPipelineLayout layout, VkRenderPass renderPass, const PipelineDescription& desc, const std::vector<Shader>& shaders)
{
	assert(desc.shaderStageCount > 0);
	VkGraphicsPipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos(desc.shaderStageCount);
	for (int i = 0; i < desc.shaderStageCount; ++i)
	{
		ShaderDescription shaderDesc = desc.shaderStages[i];
		VkPipelineShaderStageCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		createInfo.sType = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
		createInfo.pNext = nullptr;
		createInfo.stage = VkTypeConverter::from(shaderDesc.shaderStage);
		createInfo.module = shaders[i].module;
		createInfo.pName = "main";
		shaderStageCreateInfos[i] = createInfo;
	}
	createInfo.stageCount = desc.shaderStageCount;
	createInfo.pStages = shaderStageCreateInfos.data();


	VkPipelineVertexInputStateCreateInfo vertexInputState = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	for (const auto& shader : shaders)
	{
		if (shader.shaderStage == VK_SHADER_STAGE_FRAGMENT_BIT)
			continue;
		/*
		auto it = std::find_if(bindingDescriptions.begin(), bindingDescriptions.end(), 
			[&](const VkVertexInputBindingDescription& lhs) {
				return lhs.binding == shader.bindingDescriptions.binding;
			});

		if(it == bindingDescriptions.end())
		*/
		bindingDescriptions.push_back(shader.bindingDescriptions);
			
		attributeDescriptions.insert(attributeDescriptions.end(), shader.attributeDescriptions.begin(), shader.attributeDescriptions.end());
	}

	vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();

	vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();
	createInfo.pVertexInputState = &vertexInputState;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyState.topology = VkTypeConverter::from(desc.rasterizationState.topology);
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;
	createInfo.pInputAssemblyState = &inputAssemblyState;

	VkPipelineTessellationStateCreateInfo tessellationState = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
	createInfo.pTessellationState = &tessellationState;

	VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	createInfo.pViewportState = &viewportState;

	VkPipelineRasterizationStateCreateInfo rasterizerState = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizerState.depthClampEnable = VK_FALSE;
	rasterizerState.polygonMode = VkTypeConverter::from(desc.rasterizationState.polygonMode);
	rasterizerState.cullMode = VkTypeConverter::from(desc.rasterizationState.faceCulling);
	rasterizerState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerState.depthBiasEnable = VK_FALSE;
	rasterizerState.depthBiasConstantFactor = 0.0f;
	rasterizerState.depthBiasClamp = 0.0f;
	rasterizerState.depthBiasSlopeFactor = 0.0f;
	rasterizerState.lineWidth = desc.rasterizationState.lineWidth;
	createInfo.pRasterizationState = &rasterizerState;

	VkPipelineMultisampleStateCreateInfo multisampleState = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;
	createInfo.pMultisampleState = &multisampleState;

	// Moving this inside the if condition create the scope and the struct is
	// no initialized properly
	VkPipelineDepthStencilStateCreateInfo dsCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	dsCreateInfo.depthTestEnable = desc.rasterizationState.enableDepthTest;
	dsCreateInfo.depthWriteEnable = desc.rasterizationState.enableDepthWrite;
	dsCreateInfo.depthCompareOp = VkTypeConverter::from(desc.rasterizationState.depthTestFunction);
	dsCreateInfo.minDepthBounds =  0.0;
	dsCreateInfo.maxDepthBounds =  1.0;
	createInfo.pDepthStencilState = &dsCreateInfo;

	VkPipelineColorBlendAttachmentState attachment = {};
	attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (desc.blendState.enable == true)
	{
		attachment.blendEnable = VK_TRUE;
		attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		//attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		//attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	}
	else
		attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &attachment;
	colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	colorBlendStateCreateInfo.blendConstants[3] = 0.0f;
	createInfo.pColorBlendState = &colorBlendStateCreateInfo;

	VkDynamicState dynamicStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateCreateInfo.dynamicStateCount = 2;
	dynamicStateCreateInfo.pDynamicStates = dynamicStates;
	createInfo.pDynamicState = &dynamicStateCreateInfo;

	createInfo.layout = layout;
	createInfo.renderPass = renderPass;

	VkPipeline pipeline = 0;
	VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, 0, &pipeline));

	return pipeline;
}

VkPipelineLayout VulkanPipeline::create_pipeline_layout(VkDevice device, const std::vector<VkDescriptorSetLayout>& setLayouts, const std::vector<VkPushConstantRange> pushConstantRanges)
{
	VkPipelineLayoutCreateInfo createInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	createInfo.pSetLayouts = setLayouts.data();
	createInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size());
	createInfo.pPushConstantRanges = pushConstantRanges.data();
	VkPipelineLayout pipelineLayout = 0;
	VK_CHECK(vkCreatePipelineLayout(device, &createInfo, 0, &pipelineLayout));


	return pipelineLayout;
}
VkPipeline VulkanPipeline::create_compute_pipeline(VkDevice device, VkPipelineLayout layout, Shader& shader)
{
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
	shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCreateInfo.stage = shader.shaderStage;
	shaderStageCreateInfo.module = shader.module;
	shaderStageCreateInfo.pName = "main";

	VkComputePipelineCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
	createInfo.stage = shaderStageCreateInfo;
	createInfo.layout = layout;

	VkPipeline pipeline = 0;
	VK_CHECK(vkCreateComputePipelines(device, 0, 1, &createInfo, 0, &pipeline));
	return pipeline;

}
VulkanPipeline::VulkanPipeline(std::shared_ptr<VulkanAPI> m_api, const PipelineDescription& desc)
{
	VkDevice device = m_api->get_device();
	
	std::vector<Shader> shaders(desc.shaderStageCount);
	std::vector<VkPushConstantRange> pushConstantRanges;

	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (std::size_t i = 0; i < desc.shaderStageCount; ++i)
	{
		const ShaderDescription& shaderDesc = desc.shaderStages[i];
		create_shader(shaders[i], device, shaderDesc.code.c_str(), shaderDesc.sizeInByte, bindings, pushConstantRanges);
	}

	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutCreateInfo.pBindings = bindings.data();

	VkDescriptorSetLayout setLayout = 0;
	VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, 0, &setLayout));
	m_descSetLayouts.push_back(setLayout);

	VkDescriptorSetAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocateInfo.descriptorPool = m_api->get_descriptor_pool();
	allocateInfo.descriptorSetCount = static_cast<uint32_t>(m_descSetLayouts.size());
	allocateInfo.pSetLayouts = m_descSetLayouts.data();
	vkAllocateDescriptorSets(device, &allocateInfo, &m_descriptorSet);

	m_layout = create_pipeline_layout(device, m_descSetLayouts, pushConstantRanges);

	if (desc.shaderStages[0].shaderStage == ShaderStage::Compute)
	{
		m_bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		m_pipeline = create_compute_pipeline(device, m_layout, shaders[0]);
	}
	else
	{
		ASSERT_MSG(desc.renderPass != nullptr, "Undefined RenderPass in pipeline");
		m_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		m_pipeline = create_graphics_pipeline(device, m_layout, reinterpret_cast<VulkanRenderPass*>(desc.renderPass)->get_renderpass(), desc, shaders);
	}

	for (auto shader : shaders)
		vkDestroyShaderModule(device, shader.module, 0);
}

void VulkanPipeline::destroy(std::shared_ptr<VulkanAPI> m_api)
{
	VkDevice device = m_api->m_Device;
	vkDestroyPipeline(device, m_pipeline, 0);
	vkDestroyPipelineLayout(device, m_layout, 0);
	for (auto& descriptorSetLayout : m_descSetLayouts)
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, 0);

}
