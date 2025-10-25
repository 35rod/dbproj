#pragma once

#include <memory>
#include <cstdint>
#include <utility>
#include "PersistentObject.hpp"

template<typename T>
class VersionedPointer {
	static_assert(std::is_base_of<PersistentObject, T>::value, "T must derive from PersistentObject");

	uint64_t id_ = 0;
	uint64_t version_ = 0;
	std::shared_ptr<T> ptr_;

	public:
		VersionedPointer() = default;

		VersionedPointer(uint64_t id, uint64_t version, std::shared_ptr<T> ptr)
		: id_(id), version_(version), ptr_(std::move(ptr)) {}

		uint64_t id() const noexcept { return id_; }
		uint64_t version() const noexcept { return version_; }

		std::shared_ptr<T> get() const noexcept { return ptr_; }
		T* operator->() const noexcept { return ptr_.get(); }
		T& operator*() const noexcept { return *ptr_; }
		explicit operator bool() const noexcept { return static_cast<bool>(ptr_); }

		VersionedPointer<T> create_new_version() const {
			if (!ptr_) return {};

			std::unique_ptr<PersistentObject> cloned_base = ptr_->clone();

			std::shared_ptr<T> new_ptr = std::shared_ptr<T>(static_cast<T*>(cloned_base.release()));

			uint64_t new_version = version_ + 1;
			new_ptr->set_id(id_);
			new_ptr->set_version(new_version);

			return VersionedPointer<T>(id_, new_version, std::move(new_ptr));
		}
};