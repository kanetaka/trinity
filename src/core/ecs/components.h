#pragma once
#include "core/ecs/registry.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

namespace ecs
{
	struct TransformComponent
	{
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat rotation = glm::identity<glm::quat>();
		float scale = 1.0f;
		glm::mat4 world_transform = glm::mat4(1.0f);
		bool recompute = true;
	};

	struct HierarchyComponent
	{
		EntityId parent = NullEntity;
		std::vector<EntityId> children;
	};

	struct SplatDataComponent
	{
		std::string ply_file;
		// We might need to store references to Vulkan resources here or pointers to
		// classes For now, let's just keep the file path and we'll see how to
		// integrate with SplatComponent
	};

} // namespace ecs
