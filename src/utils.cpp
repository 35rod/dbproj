#include "utils.hpp"
#include <stdexcept>

namespace {
	void ensure_good_write(std::ofstream& ofs) {
		if (!ofs.good()) throw std::ios_base::failure("ofstream write failed");
	}

	void ensure_good_read(std::ifstream& ifs) {
		if (!ifs.good()) throw std::ios_base::failure("ifstream read failed");
	}
}

void write_uint64(std::ofstream& ofs, uint64_t value) {
	ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
	ensure_good_write(ofs);
}

uint64_t read_uint64(std::ifstream& ifs) {
	uint64_t value = 0;
	ifs.read(reinterpret_cast<char*>(&value), sizeof(value));
	ensure_good_read(ifs);
	return value;
}

void write_uint32(std::ofstream& ofs, uint32_t value) {
	ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
	ensure_good_write(ofs);
}

uint32_t read_uint32(std::ifstream& ifs) {
	uint32_t value = 0;
	ifs.read(reinterpret_cast<char*>(&value), sizeof(value));
	ensure_good_read(ifs);
	return value;
}

void write_int32(std::ofstream& ofs, int32_t value) {
	ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
	ensure_good_write(ofs);
}

int32_t read_int32(std::ifstream& ifs) {
	int32_t value = 0;
	ifs.read(reinterpret_cast<char*>(&value), sizeof(value));
	ensure_good_read(ifs);
	return value;
}

void write_string(std::ofstream& ofs, const std::string& s) {
	uint32_t len = static_cast<uint32_t>(s.size());
	write_uint32(ofs, len);

	if (len > 0) {
		ofs.write(s.data(), len);
		ensure_good_write(ofs);
	}
}

std::string read_string(std::ifstream& ifs) {
	uint32_t len = read_uint32(ifs);
	std::string s;

	if (len > 0) {
		s.resize(len);
		ifs.read(&s[0], len);
		ensure_good_read(ifs);
	}

	return s;
}