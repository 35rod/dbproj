#pragma once

#include <unordered_map>
#include <map>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include "PersistentObject.hpp"
#include "VersionedPointer.hpp"
#include "utils.hpp"

class DB {
	uint64_t next_id_;
	std::unordered_map<uint64_t, std::shared_ptr<PersistentObject>> store_;
	std::unordered_map<std::string, std::map<std::string, std::vector<uint64_t>>> indexes_;

	template<typename T>
	void update_indexes(uint64_t id, std::shared_ptr<T> obj) {
		auto type_name = obj->type_name();
		auto& type_indexes = indexes_[type_name];

		for (auto& [field, values] : type_indexes) {
			auto& ids = values;
			ids.erase(std::remove(ids.begin(), ids.end(), id), ids.end());
		}

		auto index_values = obj->get_index_values();
		for (const auto& [field, value] : index_values) {
			type_indexes[field].push_back(id);
		}
	}

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
			std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
			if (!ofs) throw std::runtime_error("Could not open file for writing: " + filename);

			uint64_t count = store_.size();
			write_uint64(ofs, count);

			for (const auto& [id, vp_base] : store_) {
				auto obj = static_cast<PersistentObject*>(vp_base.get());
				std::string type = obj->type_name();
				
				// Save type name
				write_string(ofs, type);
				
				// Save object data to a temporary buffer to get its size
				std::stringstream buffer;
				obj->serialize(buffer);
				std::string data = buffer.str();
				
				// Write data size followed by the data
				write_uint64(ofs, data.size());
				ofs.write(data.c_str(), data.size());
			}
		}

		template<typename T>
		void update(uint64_t id, std::shared_ptr<T> obj) {
			static_assert(std::is_base_of<PersistentObject, T>::value, "T must derive from PersistentObject");
			store_[id] = obj;
			update_indexes(id, obj);
		}

		template<typename T>
		void load(const std::string& filename) {
			static_assert(std::is_base_of<PersistentObject, T>::value, "T must derive from PersistentObject");

			std::ifstream ifs(filename, std::ios::binary);
			if (!ifs) throw std::runtime_error("Could not open file for reading: " + filename);

			ifs.seekg(0, std::ios::end);
			if (ifs.tellg() == 0) return;
			ifs.seekg(0, std::ios::beg);

			try {
				uint64_t count = read_uint64(ifs);

				for (uint64_t i = 0; i < count; i++) {
					std::string type = read_string(ifs);
					uint64_t data_size = read_uint64(ifs);
					
					if (type == T().type_name()) {
						auto obj = std::make_shared<T>();
						
						std::string data(data_size, '\0');
						if (!ifs.read(&data[0], data_size)) {
							throw std::runtime_error("Failed to read object data");
						}
						
						std::stringstream buffer(data);
						obj->deserialize(buffer);
						
						uint64_t id = obj->get_id();
						if (id >= next_id_) next_id_ = id + 1;
						
						store_[id] = obj;
						update_indexes(id, obj);
					} else {
						ifs.seekg(data_size, std::ios::cur);
					}
				}
			} catch (const std::exception& e) {
				throw std::runtime_error(std::string("Error loading database: ") + e.what());
			}
		}

		template<typename T>
		std::vector<VersionedPointer<T>> query(const std::string& field, const std::string& value) const {
			std::vector<VersionedPointer<T>> results;
			auto type_name = T().type_name();
			
			auto type_it = indexes_.find(type_name);
			if (type_it != indexes_.end()) {
				auto field_it = type_it->second.find(field);
				if (field_it != type_it->second.end()) {
					for (uint64_t id : field_it->second) {
						auto obj = get<T>(id);
						if (obj.get() && obj.get()->get_index_value(field) == value) {
							results.push_back(obj);
						}
					}
				}
			}
			
			return results;
		}

		template<typename T>
		std::vector<VersionedPointer<T>> query_range(const std::string& field, 
			const std::string& start, const std::string& end) const {
			std::vector<VersionedPointer<T>> results;
			auto type_name = T().type_name();
			
			auto type_it = indexes_.find(type_name);
			if (type_it != indexes_.end()) {
				auto field_it = type_it->second.find(field);
				if (field_it != type_it->second.end()) {
					for (uint64_t id : field_it->second) {
						auto obj = get<T>(id);
						if (!obj.get()) continue;

						if (field == "balance") {
							double value = std::stod(obj.get()->get_index_value(field));
							double start_val = std::stod(start);
							double end_val = std::stod(end);
							if (value >= start_val && value <= end_val) {
								results.push_back(obj);
							}
						} else {
							auto value = obj.get()->get_index_value(field);
							if (value >= start && value <= end) {
								results.push_back(obj);
							}
						}
					}
				}
			}
			
			return results;
		}
};