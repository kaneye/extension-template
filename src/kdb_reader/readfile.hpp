#pragma once

#include <cassert>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <limits>
#include "read.hpp"
//#include "handlers.hpp"
#include "utils.hpp"




template<typename T>
int readfile_impl(FILE* fp_, size_t file_len, size_t base_offset, const std::string& sympath, size_t item_start, size_t item_read, std::vector<T>& out);

template<typename T>
int readfile(const std::string &path, const std::string& sympath, size_t item_start, size_t item_read, std::vector<T>& out)   //, const std::string &filename, 
{
    FILE *fp_ = fopen(path.c_str(), "rb");
    size_t file_len = get_file_length(fp_);
    //std::cout <<"info,file_length:"<<file_len<<std::endl;

    readfile_impl<T>(fp_, file_len, 0, sympath, item_start, std::min(file_len-item_start,item_read), out);
    //rewind(fp_);
    //fseek(fp_, 0, SEEK_SET);
    fclose(fp_);
    return 0;
}


template<typename T>
int readfile_impl(FILE* fp_, size_t file_len, size_t base_offset, const std::string& sympath, size_t item_start, size_t item_read, std::vector<T>& out) {
    fseek(fp_, base_offset, SEEK_SET);
    std::size_t HEADER_BYTES = 2;
    std::size_t DTYPE_BYTES = 1;

    //std::vector<char> header(HEADER_BYTES, '\0');
    //fread(header.data(), 1, HEADER_BYTES, fp_);
    char header[HEADER_BYTES];    
    fread(header, 1, HEADER_BYTES, fp_);
    //std::cout<<"info,header:"<<strcmp(header,"\xfe\x20")<<","<<strcmp(header,"\xff\01")<<","<<strcmp(header,"\xFD\00")<<std::endl;   //7   
    //readlist<char>(fp_, 0, HEADER_BYTES, header);
    //std::cout<<"info,header:"<<strcmp(header.data(),"\xfe\x20")<<","<<strcmp(header.data(),"\xff\01")<<std::endl;   //7   
    //std::cout<<"info,header:"<<(std::string(header.data(),2)==std::string("\xfe\x20",2))<<","<<(std::string(header.data(),2)==std::string("\xff\01",2))<<std::endl;   //7   

    char datatype;        
    size_t read = fread(&datatype, DTYPE_BYTES, 1, fp_);
    //char datatype = readatom<char>(fp_, HEADER_BYTES, DTYPE_BYTES);
    //std::cout<<"info,datatype:"<<(int)datatype<<std::endl;   //7


    if(strncmp(header,"kx",HEADER_BYTES)==0) {  //.data()
        assert(0);
    }
    if((int)datatype<20) {
        if(strncmp(header,"\xfe\x20",HEADER_BYTES)==0) {   //list
            std::cout<<"info,header:FE20,dtype:"<<(int)datatype<<std::endl;
            size_t offset = 16+base_offset;
            size_t byte_size = file_len-offset;
            //readlist(fp_, file_offset, byte_size, std::vector<T>& out)
            read0<T>((int)datatype, fp_, offset, byte_size, item_start, item_read, out);
            return 0;
        }
        else if(strncmp(header,"\xff\01",HEADER_BYTES)==0) {  //atoms
            std::cout<<"info,header:FF01,dtype:"<<(int)datatype<<std::endl;
            size_t offset = 3+base_offset;
            size_t byte_size = file_len-offset;
            read0<T>((int)datatype, fp_, offset, byte_size, item_start, item_read, out);
            return 0;
        }
    }
    if((strncmp(header,"\xFD\00",HEADER_BYTES)==0)&&((int)datatype>=20)&&((int)datatype<=77)) {   //enum
        std::cout<<"info,header:FD00,dtype:"<<(int)datatype<<",";
        std::cout<<"This is an extension header following kdb::Parser::ExtListHeader."<<std::endl;
        std::vector<long> idx;        
        //readlist<long>(fp_, base_offset+16, file_len-(base_offset+16), idx);
        readlist1<long>(fp_, base_offset+16, file_len-(base_offset+16), item_start, item_read, idx);
        //std::cout<<"info,enum idx:"<<idx.size()<<","<<idx[0]<<","<<idx[10]<<std::endl;

        std::cout<<"info,read sym file..."<<sympath<<std::endl;
        //FILE *fpsym = fopen(sympath.c_str(), "rb")
        //size_t symfile_len = get_file_length(fpsym); 
        // std::vector<std::string> sym_vec;
        // readfile<std::string>(sympath, "", sym_vec);  
        std::vector<T> sym_vec;
        //std::cout<<"std::numeric_limits<size_t>::max():"<<std::numeric_limits<size_t>::max()<<std::endl;
        readfile<T>(sympath, "", 0, 10000000, sym_vec);  
        //std::cout<<"debug,read sym file:"<<sym_vec.size()<<std::endl;        
        for(long i=0;i<idx.size();i++)
            out.push_back(sym_vec[idx[i]]);
            
    }
    if(((int)datatype>77)&&((int)datatype<97)) {
        std::cout<<"error,not implemented"<<std::endl;
        assert(0);
    }
//   if filename.endswith("#"):                                 # hash == data file
//     return data

    if(strncmp(header,"\xfd\x20",HEADER_BYTES)==0) {
        std::cout<<"info,header:FD20,dtype:"<<(int)datatype<<",ExtListHeader"<<std::endl;
        // ExtListHeader(16)
        // char enum_name[4096 - (16 + 16)];  //就是一个字符串"sym"
        // readlist(fp_, 32, size_t file_byte_size, std::vector<T>& out)                  
        readfile_impl(fp_, file_len, 0x1000-16, sympath, item_start, item_read, out);   //skip ExtListHeader
        return 0;
    }    
   
}