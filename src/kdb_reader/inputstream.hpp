#pragma once

#include <iostream>
#include <vector>


class inputstream {
public:
    virtual size_t fread1(void *ptr, size_t size, size_t count) = 0;
    virtual int fseek1(long offset, int fromwhere) = 0;
    virtual size_t get_length() = 0;
};


class filestream:public inputstream {
public:
    size_t fread1(void *ptr, size_t size, size_t count);
    int fseek1(long offset, int fromwhere);   
    size_t get_length(); 
    //filestream(FILE* fp);
    filestream(const std::string &path);
    ~filestream();

private:
    bool init();

//private:
public:
    std::string path_;
    FILE* fp_;
};


#define byte unsigned char

class bufferstream:public inputstream {
public:
    size_t fread1(void *ptr, size_t size, size_t count);
    int fseek1(long offset, int fromwhere);   
    size_t get_length(); 
    //bufferstream(char* buffer);

public:    
    std::vector<byte> buffer_;

private:    
    //char* buffer_;
    size_t pos_;
};

