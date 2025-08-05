#include <cassert>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include "string.h"
#include "read.hpp"
//#include "handlers.hpp"
#include "utils.hpp"


/*
std::size_t get_file_length(FILE *fp) 
{
    assert(fp);
    const auto p_curr = ftell(fp);    

    fseek(fp, 0, SEEK_END);
    std::size_t len = ftell(fp);

    fseek(fp, p_curr, SEEK_SET);
    return len;
}


int read_meta(const std::string &path) 
{
    FILE *fp_ = fopen(path.c_str(), "rb");
    size_t file_len = get_file_length(fp_);

    fseek(fp_, 0, SEEK_SET);
    std::size_t HEADER_BYTES = 2;
    std::size_t DTYPE_BYTES = 1;

    char header[HEADER_BYTES];    
    fread(header, 1, HEADER_BYTES, fp_);

    char datatype;        
    size_t read = fread(&datatype, DTYPE_BYTES, 1, fp_);

    //std::cout<<strncmp(header,"\xfd\x20", HEADER_BYTES)<<","<<strcmp(header,"\xfe\x20")<<","<<strcmp(header,"\xFD\00")<<std::endl;
    if(strncmp(header,"\xfd\x20",HEADER_BYTES)==0) {
        std::cout<<"info,header:FD20,dtype:"<<(int)datatype<<",ExtListHeader"<<std::endl;
        //readfile_impl(fp_, file_len, 0x1000-16, sympath);   //skip ExtListHeader
        fseek(fp_, 0x1000-16, SEEK_SET);
        fread(header, 1, HEADER_BYTES, fp_);
        fread(&datatype, DTYPE_BYTES, 1, fp_);
        if((strncmp(header,"\xFD\00",HEADER_BYTES)==0)&&((int)datatype>=20)&&((int)datatype<=77)) {   //enum
            return 11;
        }
        else {
            std::cout<<"error,not enum type after ExtListHeader"<<std::endl;
            assert(0);
        }
    }

    fclose(fp_);
    return (int)datatype;
}
*/

/*
int readfile(const std::string &path, const std::string& sympath)   //, const std::string &filename, 
{
    FILE *fp_ = fopen(path.c_str(), "rb");
    size_t file_len = get_file_length(fp_);
    //std::cout <<"info,file_length:"<<file_len<<std::endl;

    readfile_impl(fp_, file_len, 0, sympath);
    //rewind(fp_);
    //fseek(fp_, 0, SEEK_SET);
    fclose(fp_);
    return 0;
}


int readfile_impl(FILE* fp_, size_t file_len, size_t base_offset, const std::string& sympath) {
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


    if(strcmp(header,"kx")==0) {  //.data()
        assert(0);
    }
    if((int)datatype<20) {
        if(strcmp(header,"\xfe\x20")==0) {   //list
            std::cout<<"info,header:FE20,dtype:"<<(int)datatype<<std::endl;
            size_t offset = 16+base_offset;
            size_t byte_size = file_len-offset;
            //readlist(fp_, file_offset, byte_size, std::vector<T>& out)
            read_by_dtype((int)datatype, fp_, offset, byte_size);
            return 0;
        }
        else if(strcmp(header,"\xff\01")==0) {  //atoms
            std::cout<<"info,header:FF01,dtype:"<<(int)datatype<<std::endl;
            size_t offset = 3+base_offset;
            size_t byte_size = file_len-offset;
            read_by_dtype((int)datatype, fp_, offset, byte_size);
            return 0;
        }
    }
    if((strcmp(header,"\xFD\00")==0)&&((int)datatype>=20)&&((int)datatype<=77)) {   //enum
        std::cout<<"info,header:FD00,dtype:"<<(int)datatype<<",";
        std::cout<<"This is an extension header following kdb::Parser::ExtListHeader."<<std::endl;
        //read_by_dtype((int)datatype, fp_, offset, byte_size);
        std::vector<long> idx;        
        readlist<long>(fp_, base_offset+16, file_len-(base_offset+16), idx);
        std::cout<<"info,enum idx:"<<idx.size()<<","<<idx[0]<<","<<idx[10]<<std::endl;

        std::cout<<"info,read sym file..."<<sympath<<std::endl;
        //FILE *fpsym = fopen(sympath.c_str(), "rb")
        //size_t symfile_len = get_file_length(fpsym);        
        readfile(sympath, "");  
        //read_by_dtype(int dtype, FILE* fp, size_t file_offset, size_t byte_size);
        //fclose(fpsym);
    }
    if(((int)datatype>77)&&((int)datatype<97)) {
        std::cout<<"error,not implemented"<<std::endl;
        assert(0);
    }
//   if filename.endswith("#"):                                 # hash == data file
//     return data

    if(strcmp(header,"\xfd\x20")==0) {
        std::cout<<"info,header:FD20,dtype:"<<(int)datatype<<",ExtListHeader"<<std::endl;
        // ExtListHeader(16)
        // char enum_name[4096 - (16 + 16)];  //就是一个字符串"sym"
        // readlist(fp_, 32, size_t file_byte_size, std::vector<T>& out)                  
        readfile_impl(fp_, file_len, 0x1000-16, sympath);   //skip ExtListHeader
        return 0;
    }    
   
}
*/

