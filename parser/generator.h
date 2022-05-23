#pragma once
#include <filesystem>

// Forward declarations
struct ParsedData;
namespace fs = std::filesystem;


// Reflection generator
// ----------------------------------------------------------------------------------------------
void generate_file_reflection_info   (const fs::path& path, ParsedData& parsing_data);
void generate_file_reflection_header (const std::vector<fs::path>& files);