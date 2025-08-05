#pragma once

#include <iostream>
#include <string>
#include <vector>


int split_string(const std::vector<char>& str, std::vector<std::string>& r_vec);
std::vector<std::string> split_string(const std::string& s, const std::string& delim);
std::string trim(const std::string &s);

void list_files(const std::string& path, std::vector<std::string>& filenames);
bool is_directory(const char* path);
bool file_exist(const char* path);
