#include "PersistentObject.hpp"

PersistentObject::PersistentObject()
: id_(0), version_(0) {}

PersistentObject::PersistentObject(uint64_t id, uint64_t version)
: id_(id), version_(version) {}

PersistentObject::~PersistentObject() = default;

uint64_t PersistentObject::get_id() const noexcept {
	return id_;
}

uint64_t PersistentObject::get_version() const noexcept {
	return version_;
}

void PersistentObject::set_id(uint64_t id) noexcept {
	id_ = id;
}

void PersistentObject::set_version(uint64_t version) noexcept {
	version_ = version;
}