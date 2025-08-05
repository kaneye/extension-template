#include <dirent.h>
#include <cstring>
#include <algorithm>
#include "utils.hpp"
#include <sys/stat.h>
#include <stdio.h>
#include <cstdio>



int split_string(const std::vector<char>& str, std::vector<std::string>& r_vec) {
    auto start = str.begin();
    auto end = str.end();     
    while (start != end) {
        auto found = std::find(start, end, '\0');
        if(found != end) {
            r_vec.emplace_back(start, found);
            start = std::next(found);
        } else 
            start = end;
    }
    // if (!r_vec.empty() && r_vec.back().empty()) {
    //     r_vec.pop_back();
    // }

/*
    char* split = strtok(str.data(), "\0");
    while(split!=NULL) {
        //std::string tmp(split);
        //std::cout<<split<<std::endl;
        //r_vec.push_back(tmp);
        split = strtok(NULL, "\0");
    }
*/    
    return 0;
}



std::vector<std::string> split_string(const std::string& s, const std::string& delim) {
    std::vector<std::string> tokens;
    size_t pos = 0, next;
    while ((next = s.find(delim, pos)) != std::string::npos) {
        tokens.push_back(s.substr(pos, next - pos));
        pos = next + delim.length();
    }
    tokens.push_back(s.substr(pos));  // 添加剩余部分
    return tokens;
}

std::string trim(const std::string &s) {
    auto start = s.find_first_not_of(" \t\r\n");
    auto end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}



void list_files(const std::string& path, std::vector<std::string>& filenames) {
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        std::cerr << "error,Error opening directory: " << path << std::endl;
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            filenames.push_back(entry->d_name);
        }
    }
    closedir(dir);
}


bool is_directory(const char* path) {
    struct stat info;
    if (stat(path, &info) != 0) 
        return false;  // 路径无效
    return S_ISDIR(info.st_mode);              // 判断是否为目录
}


bool file_exist(const char* path) {
    FILE* file = fopen(path, "r");
    if (file) { 
        fclose(file); 
        return true; 
    }
    return false;
}


void test_list_files() {
    std::string path("/home/yky/tmp/lists/"); 
    std::vector<std::string> files;    
    list_files(path, files);
    for(size_t i=0; i<files.size();i++)
        std::cout<<files[i]<<std::endl;
}

