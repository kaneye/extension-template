#pragma once

#include <cassert>
#include <iostream>
#include <vector>
#include <memory> 
#include <typeinfo>
#include "string.h"
#include <string>
#include "inputstream.hpp"
#include "utils.hpp"


template<typename T>
T readatom(std::shared_ptr<inputstream>& istream, size_t file_offset, size_t file_byte_size)
{
    size_t sizeof_dtype = sizeof(T);
    assert(file_byte_size==sizeof_dtype);
    // std::vector<unsigned char> buffer(sizeof_dtype, '\0'); 
    // unsigned char *pbuffer = buffer.data();
    unsigned char pbuffer[sizeof_dtype];

    // fseek(fp, file_offset, SEEK_SET);
    // size_t bytes_read = fread(pbuffer, 1, sizeof_dtype, fp);
    istream->fseek1(file_offset, SEEK_SET);
    size_t bytes_read = istream->fread1(pbuffer, 1, sizeof_dtype);
    assert(bytes_read==sizeof_dtype);
    return *((T*)pbuffer);
    //T tmp = *((T*)pbuffer);
    //pbuffer = NULL;
    //return tmp;
}


template<typename T>
int readlist(std::shared_ptr<inputstream>& istream, size_t file_offset, size_t file_byte_size, std::vector<T>& out)
{
    size_t sizeof_dtype = sizeof(T);
    size_t list_size = file_byte_size/sizeof_dtype;
    size_t byte_to_read = sizeof_dtype*list_size;
    assert(byte_to_read==file_byte_size);

    //std::vector<unsigned char> buffer(file_byte_size, '\0');
    //assert(buffer.size()==file_byte_size);
    //unsigned char *pbuffer = buffer.data();
    unsigned char* pbuffer = (unsigned char*)malloc(file_byte_size);
    if(pbuffer==NULL) {
        std::cout<<"error,memory allocation failed"<<std::endl;
        return -1;
    }

    //fseek(fp, file_offset, SEEK_SET);
    //size_t bytes_read = fread(pbuffer, 1, file_byte_size, fp);
    istream->fseek1(file_offset, SEEK_SET);
    size_t bytes_read = istream->fread1(pbuffer, 1, file_byte_size);
    assert(bytes_read==file_byte_size);

    T* psrc = (T*)pbuffer;
    //std::vector<T> tgt_vec(psrc,psrc+list_size);
    out.assign(psrc,psrc+list_size);    
    //std::cout <<"info,"<<tgt_vec[0]<<","<<tgt_vec[10]<<std::endl;
    psrc = NULL;
    free(pbuffer);
    return 0;
}


template<typename T>
int readlist1(std::shared_ptr<inputstream>& istream, size_t file_offset, size_t file_byte_size, size_t item_start, size_t item_read, std::vector<T>& out)
{
    //std::cout<<"debug,readlist1:"<<item_start<<","<<item_read<<","<<file_byte_size<<std::endl;
    size_t sizeof_dtype = sizeof(T);
    size_t list_size = file_byte_size/sizeof_dtype;
    item_read = std::min(list_size-item_start,item_read);
    if(item_read<=0)
        return 0;    
        
    //std::cout<<"debug,item_read:"<<item_read<<std::endl;
    size_t buffer_size = item_read*sizeof_dtype;
    //assert(file_byte_size>=buffer_size);

    unsigned char* pbuffer = (unsigned char*)malloc(buffer_size);
    if(pbuffer==NULL) {
        std::cout<<"error,memory allocation failed"<<std::endl;
        return -1;
    }

    // fseek(fp, file_offset+item_start*sizeof_dtype, SEEK_SET);
    // size_t bytes_read = fread(pbuffer, 1, buffer_size, fp);
    istream->fseek1(file_offset+item_start*sizeof_dtype, SEEK_SET);
    size_t bytes_read = istream->fread1(pbuffer, 1, buffer_size);
    assert(bytes_read==buffer_size);

    T* psrc = (T*)pbuffer;
    //std::vector<T> tgt_vec(psrc,psrc+item_read);
    out.assign(psrc,psrc+item_read);    
    //std::cout <<"info,"<<tgt_vec[0]<<","<<tgt_vec[10]<<std::endl;
    psrc = NULL;
    free(pbuffer);
    return 0;
}



// template<int absdtype, typename T>
// typename std::enable_if<absdtype!=11,int>::type 
// read0(int dtype, FILE *fp, size_t file_offset, size_t file_byte_size, std::vector<T>& out) 
template<typename T>
int read0(int dtype, std::shared_ptr<inputstream>& istream, size_t file_offset, size_t file_byte_size, size_t item_start, size_t item_read, std::vector<T>& out)
{
    if(dtype==0) {
        std::cout<<"error,unknown datatype 0.TODO..."<<std::endl;

    } else if(abs(dtype)==2) {
        std::cout<<"error,unknown datatype 2/-2.TODO..."<<std::endl;
    
    } else {
        if(dtype<0) {
            T r=readatom<T>(istream, file_offset, file_byte_size);
            //return r;
            //std::cout<<"info,Kdtype:"<<dtype<<",dtype:"<<typeid(r).name()<<",value:"<<r<<std::endl;
            out.push_back(r);

        } else if(dtype>0) {
            //std::vector<T> r_vec;
            //readlist<T>(fp, file_offset, file_byte_size, r_vec);
            //std::cout<<"info,Kdtype:"<<dtype<<",dtype:"<<typeid(r_vec[0]).name()<<",size:"<<r_vec.size()<<",someval:"<<r_vec[5]<<std::endl;
            //if(r_vec.size()<20) {
            // if(1) {
            //     for(size_t i=0;i<20;i++)
            //         std::cout<<r_vec[i]<<std::endl;
            // }   

            readlist1<T>(istream, file_offset, file_byte_size, item_start, item_read, out);
        }
    }
    return 0;
}


// template<int absdtype, typename T>
// typename std::enable_if<absdtype==11,int>::type 
// read0(int dtype, FILE *fp, size_t file_offset, size_t file_byte_size, std::vector<T>& out)
template<>
inline int read0(int dtype, std::shared_ptr<inputstream>& istream, size_t file_offset, size_t file_byte_size, size_t item_start, size_t item_read, std::vector<std::string>& out)
{
    if(dtype==-11) {
        std::vector<char> buffer(file_byte_size, '\0'); 
        readlist<char>(istream, file_offset, file_byte_size-1, buffer); 
        std::string r(buffer.data());
        //std::cout<<"info,Kdtype:"<<dtype<<",dtype:"<<"string"<<",value:"<<(char*)buffer.data()<<std::endl;
        out.push_back(r);

    } else if(dtype==11) {
        //std::cout<<"debug,dtype=11,"<<file_offset<<","<<file_byte_size<<std::endl;
        size_t additional_offset = 5;
        std::vector<char> buffer(file_byte_size-additional_offset, '\0'); 
        readlist<char>(istream, file_offset+additional_offset, file_byte_size-additional_offset, buffer); 
        //std::cout<<"debug,dtype=11,"<<buffer.size()<<","<<buffer[0]<<buffer[1]<<buffer[2]<<buffer[3]<<buffer[4]<<buffer[5]<<","<<buffer[-1]<<std::endl;

/*
        std::vector<std::string> r_vec;
        split_string(buffer, r_vec);            
        std::cout<<"info,Kdtype:"<<dtype<<",dtype:"<<"string"<<",size:"<<r_vec.size()<<",someval:"<<r_vec[5]<<","<<r_vec[r_vec.size()-1]<<std::endl;
        if(r_vec.size()<20) {
            for(size_t i=0;i<r_vec.size();i++)
                std::cout<<r_vec[i]<<std::endl;
        }
*/   
        //split_string(buffer, out);   //original impl
        std::vector<std::string> tmp;
        split_string(buffer, tmp); 
        //std::cout<<"debug,after split_string:"<<tmp.size()<<std::endl;
        assert(item_start<=tmp.size());
        item_read = std::min(tmp.size()-item_start,item_read);
        //std::cout<<"debug,before assign:"<<item_start<<","<<item_start+item_read<<std::endl;
        out.assign(tmp.begin()+item_start, tmp.begin()+item_start+item_read);
        //std::cout<<"debug,after assign:"<<out.size()<<std::endl;

    } else {
        std::cout<<"error,wrong dtype in template function"<<std::endl;
        assert(0);
    }
    return 0;
}


//std::size_t get_file_length(FILE *fp);

//int read_meta(const std::string &path);

//int readfile_impl(FILE* fp_, size_t file_len, size_t base_offset=0, const std::string& sympath="");