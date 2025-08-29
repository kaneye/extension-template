#include "inputstream.hpp"
#include <string.h>
#include <cassert>



bool filestream::init() {
    if(this->path_=="") {
        std::cout<<"error,file path is empty:"<<this->path_<<std::endl;
        return false;
    }        

    this->fp_ = fopen(this->path_.c_str(), "rb");
    if (this->fp_==NULL)
        return false;

    return true;
}
size_t filestream::fread1(void *ptr, size_t size, size_t count) {
    if(fp_==nullptr)
        this->init();

    assert(fp_);
    return fread(ptr, size, count, this->fp_);
}
int filestream::fseek1(long offset, int fromwhere) {
    if(fp_==nullptr)
        this->init();

    assert(fp_);
    return fseek(this->fp_, offset, fromwhere);
}
size_t filestream::get_length() {
    if(fp_==nullptr)
        this->init();

    assert(fp_);
    const auto p_curr = ftell(fp_);    

    fseek(fp_, 0, SEEK_END);
    std::size_t len = ftell(fp_);

    fseek(fp_, p_curr, SEEK_SET);
    return len;
}
//filestream::filestream(FILE* fp): fp_(fp) {}
filestream::filestream(const std::string &path): path_(path),fp_(nullptr) {}
filestream::~filestream() {
    fclose(this->fp_);
    std::cout<<"debug,~filestream()"<<std::endl;
}



size_t bufferstream::fread1(void *ptr, size_t size, size_t count) {
    assert(this->get_length()>0);
    assert(this->get_length()>=this->pos_);
    size_t bytes_read = std::min(size*count, get_length()-this->pos_);
    //std::cout<<"mem_debug,bufferstream::fread1,before memcpy:"<<pos_<<","<<ptr<<","<<bytes_read;
    memcpy(ptr,this->buffer_.data()+this->pos_, bytes_read);
    //std::cout<<"mem_debug,bufferstream::fread1,after memcpy";
    this->pos_ += bytes_read;
    return bytes_read;
}
size_t bufferstream::get_length() {
    return buffer_.size();
}
int bufferstream::fseek1(long offset, int fromwhere) {
    if(SEEK_SET==fromwhere)
        this->pos_ = 0+offset;
    else {
        std::cout<<"error,feek1"<<std::endl;
        assert(0);
    }
}
//bufferstream::bufferstream(char* buffer): buffer_(buffer) { pos_ = 0;}
