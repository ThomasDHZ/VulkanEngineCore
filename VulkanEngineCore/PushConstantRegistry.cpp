//#include "PushConstantRegistry.h"
//
//PushConstantRegistry& pushConstantRegistry = PushConstantRegistry::Get();
//
//void PushConstantRegistry::RegisterPushConstantValue(const String& sourceName, UpdateFunc func)
//{
//	registry[sourceName] = std::move(func);
//}
//
//void PushConstantRegistry::ApplyPushConstantRules(ShaderPushConstant& pushConstant, const PushConstantContext& pushConstantContext)
//{
//	auto it = registry.find(pushConstant.PushConstantName);
//	if (it != registry.end())
//	{
//		it->second(pushConstant, pushConstantContext);
//	}
//}