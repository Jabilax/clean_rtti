#pragma once
#include <filesystem>

// Forward declarations.
struct ParsedData;
namespace fs = std::filesystem;

void generate_reflection_info(const fs::path& file_path, const ParsedData& data);