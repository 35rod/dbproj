#pragma once
#include <fstream>
#include <string>
#include <cstdint>

void write_uint64(std::ofstream& ofs, uint64_t value);
uint64_t read_uint64(std::ifstream& ifs);

void write_uint32(std::ofstream& ofs, uint32_t value);
uint32_t read_uint32(std::ifstream& ifs);

void write_int32(std::ofstream& ofs, int32_t value);
int32_t read_int32(std::ifstream& ifs);

void write_string(std::ofstream& ofs, const std::string& s);
std::string read_string(std::ifstream& ifs);