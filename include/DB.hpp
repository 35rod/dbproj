#pragma once

#include <unordered_map>
#include <memory>
#include "PersistentObject.hpp"
#include "VersionedPointer.hpp"
#include "utils.hpp"

class DB {
	uint64_t next_id_;
	std::unordered_map<uint64_t, std::shared_ptr<PersistentObject>> store_;

	public:
		DB() : next_id_(1) {}

		template<typename T>
		uint64_t add(std::shared_ptr<T> obj) {
			static_assert(std::is_base_of<PersistentObject, T>::value, "T must derive from PersistentObject");

			uint64_t id = next_id_++;
			obj->set_id(id);
			obj->set_version(0);

			store_[id] = obj;

			return id;
		}

		template<typename T>
		VersionedPointer<T> get(uint64_t id) const {
			auto it = store_.find(id);

			if (it != store_.end()) {
				std::shared_ptr<T> cast_ptr = std::dynamic_pointer_cast<T>(it->second);

				if (cast_ptr) {
					return VersionedPointer<T>(cast_ptr->get_id(), cast_ptr->get_version(), cast_ptr);
				}
			}

			return VersionedPointer<T>();
		}

		void remove(uint64_t id) {
			store_.erase(id);
		}

		void save(const std::string& filename) const {
			std::ofstream ofs(filename, std::ios::binary);
			if (!ofs) return;

			uint64_t count = store_.size();
			write_uint64(ofs, count);

			for (auto& [id, vp_base] : store_) {
				auto obj = static_cast<PersistentObject*>(vp_base.get());
				std::string type = obj->type_name();
				write_string(ofs, type);
				obj->serialize(ofs);
			}
		}

		template<typename T>
		void update(uint64_t id, std::shared_ptr<T> obj) {
			static_assert(std::is_base_of<PersistentObject, T>::value, "T must derive from PersistentObject");
			store_[id] = obj;
		}

		template<typename T>
		void load(const std::string& filename) {
			static_assert(std::is_base_of<PersistentObject, T>::value, "T must derive from PersistentObject");

			std::ifstream ifs(filename, std::ios::binary);
			if (!ifs) return;

			uint64_t count = read_uint64(ifs);

			for (uint64_t i = 0; i < count; i++) {
				std::string type = read_string(ifs);

				if (type == T().type_name()) {
					auto obj = std::make_shared<T>();
					obj->deserialize(ifs);

					uint64_t id = obj->get_id();
					if (id >= next_id_) next_id_ = id + 1;

					store_[id] = obj;
				}
			}
		}
};