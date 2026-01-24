#pragma once

#include <Event.hpp>

#include <typeindex>
#include <unordered_map>
#include <functional>
#include <memory>

namespace Lithe {

class EventDispatcher {

	private:

		template<typename T>
		using Callback = std::function<bool(T)>;

	public:

		template<typename T>
		void on(Callback<T> callback) {
			auto& vec = mSubscribers[typeid(T)];
			vec.push_back(std::make_shared<Callback<T>>(std::move(callback)));
		}

		template<typename T>
		void dispatch(T&& event) {
			auto it = mSubscribers.find(typeid(std::decay_t<T>));
			if (it != mSubscribers.end()) {
				for (auto ptr : it->second) {
					auto* cb = static_cast<Callback<std::decay_t<T>>*>(ptr.get());
					if (!(*cb)(std::forward<T>(event))) 
						break; // stop if callback returns true
				}
			}
		}


	private:
		
		std::unordered_map<
			std::type_index,
			std::vector<std::shared_ptr<void>>
		> mSubscribers;
    
};

}
