#pragma once

#include "Lithe/Scene/Scene.h"
#include "Lithe/Scene/Components.h"
#include <array>
#include <cstdint>

namespace Lithe {

	class RenderSystem {

		template<class... Components>
		struct EntityBuffer {
			static constexpr std::size_t MAX_ENTITIES = 100;

			alignas(16) std::array<uint8_t, (sizeof(Components[MAX_ENTITIES]) + ...)> buffer;
			std::array<int, sizeof...(Components)> strides;
			std::size_t entityCount = 0;

			EntityBuffer() {
				strides = { (sizeof(Components[MAX_ENTITIES]))... };
			}

			template<class T>
			T* getData(size_t componentIndex = 0) {
				size_t offset = 0;
				
				// Calculate offset to the component array we want
				for (size_t i = 0; i < componentIndex; i++)
					offset += strides[i];

				return reinterpret_cast<T*>(buffer.data() + offset);
			}

			void push(const Components&... args) {
				if (entityCount >= MAX_ENTITIES) return;

				// Unpack components into correct arrays
				size_t idx = 0;
				((copyComponent(args, idx++)), ...);

				entityCount++;
			}

			private:

				template<class T>
				void copyComponent(const T& component, size_t componentIdx) {
					T* arr = getData<T>(componentIdx);
					arr[entityCount] = component;
				}


		};


		public:


			static auto buildEntityBuffer(const Scene& scene) {

				EntityBuffer<TransformComponent, SpriteComponent> buffer;

				for (const auto& entity : scene) {
					auto& transform = entity->getOrCreate<TransformComponent>();
					auto& sprite = entity->getOrCreate<SpriteComponent>();

					buffer.push(transform, sprite);

				}

				return buffer;

			}


	};

}