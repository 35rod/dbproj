#pragma once
#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

class PersistentObject {
	uint64_t id_;
	uint64_t version_;

	public:
		PersistentObject();
		PersistentObject(uint64_t id, uint64_t version);
		virtual ~PersistentObject();

		uint64_t get_id() const noexcept;
		uint64_t get_version() const noexcept;

		void set_id(uint64_t id) noexcept;
		void set_version(uint64_t version) noexcept;
		
		virtual void serialize(std::ostream& os) const = 0;
		virtual void deserialize(std::istream& is) = 0;
		virtual std::unique_ptr<PersistentObject> clone() const = 0;
		virtual std::string type_name() const = 0;

		virtual std::unordered_map<std::string, std::string> get_index_values() const {
			return {};
		}

		virtual std::string get_index_value(const std::string& field) const {
			auto values = get_index_values();
			auto it = values.find(field);
			return it != values.end() ? it->second : "";
		}
};