#include "kdbreader.hpp"
#include "qfile.hpp"
//#include <limits>
#include <fstream>
#include "readstring.hpp"

#define DEBUG_MODE 1


// KDBFileReader::KDBFileReader(const std::string &path, const std::string& sympath)
// {

// }

KDBFileReader::KDBFileReader(const std::string &path, const std::string& sympath) 
: path_(path), sympath_(sympath),readed_size_(0),total_size_(0),finished_(false)
{
    //this->istream_ = std::make_shared<filestream>(path);
    this->istream_ = std::make_shared<bufferstream>();
};

KDBFileReader::~KDBFileReader() {
//    fclose(this->fp_);
}
void KDBFileReader::set_istream(std::shared_ptr<inputstream> istream)
{
    this->istream_ = istream;
}


int KDBFileReader::read_meta(size_t base_offset) {
#if DEBUG_MODE 
    std::cout<<"info,read_meta:"<<this->path_<<",base_offset:"<<base_offset<<std::endl;
#endif    

    FILE* fp = fopen(this->path_.c_str(), "rb");
    fseek(fp, base_offset, SEEK_SET);   
    char datatype; 

    if(true) { 
#if DEBUG_MODE 
            std::cout<<"debug,kxzipped"<<std::endl;
#endif
            std::shared_ptr<bufferstream> buffer_stream = std::dynamic_pointer_cast<bufferstream>(this->istream_); 
            if(true) {
            kdb::BinFile binfile = kdb::BinFile(fp);
            binfile.inflateBody(buffer_stream->buffer_);  
            //std::cout<<"info,inflateBody:"<<buffer_stream->buffer_.size()<<std::endl;
#if DEBUG_MODE     
            std::cout<<"mem_debug,inflateBody completed"<<std::endl;
#endif        
            }
            
            //return this->read_meta(0);
            if(true) {
                std::cout<<"mem_debug,read header,dtype from new stream"<<std::endl;
                this->file_len_ = this->istream_->get_length(); 
                std::cout<<"mem_debug,file_len_:"<<this->file_len_<<std::endl;
                this->istream_->fseek1(0, SEEK_SET);
                std::cout<<"mem_debug,fseek1:"<<std::endl;
                size_t read_byte = this->istream_->fread1(this->header,1, HEADER_BYTES);            
                std::cout<<"mem_debug,fread header:"<<read_byte<<std::endl;
                size_t read_byte = this->istream_->fread1(&datatype, DTYPE_BYTES, 1);
                std::cout<<"mem_debug,fread datatype:"<<read_byte<<std::endl;
                this->dtype_ = (int)datatype;
                std::cout<<"mem_debug,read header,dtype from new stream completed."<<std::endl;
            }
        }
  
    if((int)datatype<20) {
        if(strncmp(header,"\xfe\x20",HEADER_BYTES)==0) {   //list
#if DEBUG_MODE         
            std::cout<<"info,header:FE20,dtype:"<<(int)datatype<<std::endl;
#endif            
            this->offset_ = 16+base_offset;
            this->file_byte_size_ = this->file_len_-this->offset_;
            //read0<T>((int)datatype, fp_, offset, byte_size, item_start, item_read, out);
            return 0;
        }
        else if(strncmp(header,"\xff\01",HEADER_BYTES)==0) {  //atoms
#if DEBUG_MODE         
            std::cout<<"info,header:FF01,dtype:"<<(int)datatype<<std::endl;
#endif            
            this->offset_ = 3+base_offset;
            this->file_byte_size_ = this->file_len_-this->offset_;
            //read0<T>((int)datatype, fp_, offset, byte_size, item_start, item_read, out);
            return 0;
        }
    }
    if((strncmp(header,"\xFD\00",HEADER_BYTES)==0)&&((int)datatype==4)) {
#if DEBUG_MODE     
        std::cout<<"info,header:FD00,dtype:"<<(int)datatype<<std::endl;
#endif                
        this->offset_ = 16+base_offset;
        this->file_byte_size_ = this->file_len_-this->offset_;
        return 0;

    }
    if((strncmp(header,"\xFD\00",HEADER_BYTES)==0)&&((int)datatype>=20)&&((int)datatype<=76)) {   //enum
#if DEBUG_MODE     
        std::cout<<"info,header:FD00,dtype:"<<(int)datatype<<",";
        std::cout<<"This is an extension header following kdb::Parser::ExtListHeader."<<std::endl;
#endif        
        this->offset_ = base_offset+16;
        this->file_byte_size_ = this->file_len_ - this->offset_;

        ///comments:read item_cnt from header
        std::vector<uint8_t> as_item_cnt;
        readlist<uint8_t>(this->istream_, base_offset+8, 8, as_item_cnt);
        long item_cnt = 0;
        for (int i=7; i>=0; i--) {
            item_cnt = (item_cnt << 8) | as_item_cnt[i];
        }

        if(this->file_byte_size_ != sizeof(long)*item_cnt) {
#if DEBUG_MODE             
            std::cout<<"info,file_size not match:item_cnt:"<<std::dec<<item_cnt<<",file_byte:"<<this->file_byte_size_<<std::endl;
#endif
            if(item_cnt>0)
                this->file_byte_size_ = sizeof(long)*item_cnt; 
        }  
        
        //std::vector<long> idx; 
        //readlist1<long>(fp_, base_offset+16, file_len-(base_offset+16), item_start, item_read, idx);
        //std::cout<<"info,enum idx:"<<idx.size()<<","<<idx[0]<<","<<idx[10]<<std::endl;

        if(this->sym_vec_.empty()) {
#if DEBUG_MODE             
            std::cout<<"info,read sym file..."<<sympath_<<std::endl;  
#endif
            KDBFileReader sym_reader = KDBFileReader(this->sympath_, "");
            sym_reader.read_meta();
            sym_reader.read<std::string>(0, 10000000, this->sym_vec_);   //read all,std::numeric_limits<size_t>::max()
#if DEBUG_MODE             
            std::cout<<"info,read sym file completed:"<<std::dec<<this->sym_vec_.size()<<" ------------------------"<<std::endl; 
#endif            
            assert(this->sym_vec_.size()>0);
        }
        return 0;
    }
    if((strncmp(header,"\xFD\01",HEADER_BYTES)==0)&&((int)datatype==77)) {   //anymap
#if DEBUG_MODE  
        std::cout<<"info,header:FD01,dtype:"<<(int)datatype<<",basestrfile:anymap"<<std::endl;
#endif
        this->offset_ = base_offset+16;
        this->file_byte_size_ = this->file_len_ - this->offset_;

/*
        std::vector<long> nested_idx_vec;
        this->read<long>(0,4,nested_idx_vec);
        std::cout<<"debug,nested_idx_vec:"<<nested_idx_vec.size()<<std::endl;
        for(size_t i=0;i<nested_idx_vec.size();i++)
            std::cout<<nested_idx_vec[i]<<std::endl;

#if DEBUG_MODE          
        std::cout<<"info,read # file..."<<std::endl;  
#endif
        this->nested_ = std::make_shared<KDBFileReader>(this->path_+"#", "");
        return this->nested_->read_meta(); 
*/

        if(!this->str_vec_.empty())
            return 0;

        int num_strings = 0;
        char** string_array = NULL;
        // 调用函数读取字符串数组
        string_array = readStringArray(this->path_.c_str(), &num_strings);
        if (string_array) {
#if DEBUG_MODE             
            printf("info,Read %d strings:\n", num_strings);
#endif
            for (int i = 0; i < num_strings; ++i) {
                //printf("[%d]: %s\n", i, string_array[i]);
                this->str_vec_.push_back(string_array[i]);
            }
            // 释放内存
            freeStringArray(string_array, num_strings);
        } else {
            printf("Failed to read string array. Please ensure the files exist and are correctly formatted.\n");
        }
        return 0;
    }    
    if(((int)datatype>77)&&((int)datatype<97)) {
        std::cout<<"error,not implemented"<<std::endl;
        assert(0);
    }
//   if filename.endswith("#"):                                 # hash == data file
//     return data

    //std::cout << "debug,position6:"<<std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(header[0])) << " " <<std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(header[1]))<<std::endl;
    //std::cout<<"debug,HEADER_BYTES:"<<HEADER_BYTES<<std::endl;
    //std::cout<<"debug,strncmp:"<<std::dec<<strncmp(header,"\xfd\x20",HEADER_BYTES)<<std::endl;
    if(strncmp(header,"\xfd\x20",HEADER_BYTES)==0) {   //可能是enum类型，也有可能是nested类型
#if DEBUG_MODE         
        std::cout<<"info,header:FD20,dtype:"<<(int)datatype<<",ExtListHeader"<<std::endl;
#endif        
        // ExtListHeader(16)
        // char enum_name[4096 - (16 + 16)];  //就是一个字符串"sym"
        return this->read_meta(0x1000-16);
    }
        
}
