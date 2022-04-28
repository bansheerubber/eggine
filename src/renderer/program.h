#pragma once

#ifndef __switch__
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#endif

#include <array>
#include <tsl/robin_map.h>
#include <string>
#include <vector>

#include "memory.h"
#include "vulkanPipeline.h"

namespace std {
	template<>
	struct hash<pair<std::string, uint64_t>> {
		size_t operator()(pair<std::string, uint64_t> const& source) const noexcept {
			uint64_t result = hash<std::string>{}(source.first);
			return result ^ (source.second + 0x9e3779b9 + (result << 6) + (result >> 2));
    }
	};

	template<>
	struct equal_to<pair<std::string, uint64_t>> {
		bool operator()(const pair<std::string, uint64_t>& x, const pair<std::string, uint64_t>& y) const {
			return x.first == y.first && x.second == y.second;
		}
	};
};

namespace render {
	#ifndef __switch__
	struct DescriptorSetPair {
		std::array<vk::DescriptorSet, 2> sets;
		bool initialized = false;
	};

	struct UniformBufferPair {
		std::array<Piece*, 2> pieces;
		bool initialized = false;
	};
	#endif
	
	class Program {
		#ifndef __switch__
		friend VulkanPipeline;
		#endif
		friend class State;
		friend class Window;

		public:
			Program(class Window* window);
			void compile();
			void addShader(class Shader* shader);
		
		protected:
			std::vector<class Shader*> shaders;
			Window* window = nullptr;

			tsl::robin_map<std::string, unsigned int> uniformToBinding;

			#ifdef __switch__
			tsl::robin_map<std::string, Piece*> uniformToPiece;
			#else
			GLuint program = GL_INVALID_INDEX;
			tsl::robin_map<std::string, GLuint> uniformToBuffer;
			tsl::robin_map<std::pair<std::string, uint64_t>, UniformBufferPair> uniformToVulkanBuffer;
			tsl::robin_map<std::string, uint32_t> uniformToShaderBinding;
			tsl::robin_map<std::string, bool> isUniformSampler;
			tsl::robin_map<std::string, class Texture*> uniformToTexture;

			std::vector<vk::PipelineShaderStageCreateInfo> stages = std::vector<vk::PipelineShaderStageCreateInfo>(2);
			uint8_t stageCount = 0;
			vk::DescriptorSetLayout descriptorLayout;
			std::vector<DescriptorSetPair> descriptorSets;

			bool compiled = false;

			static unsigned int UniformCount;

			void createDescriptorSet(uint32_t index);
			vk::DescriptorSet &getDescriptorSet(uint32_t index, uint32_t framePingPong);
			#endif

			void createUniformBuffer(std::string uniformName, unsigned int size, unsigned int index = 0);
	};
};
