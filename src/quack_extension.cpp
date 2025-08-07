#define DUCKDB_EXTENSION_MAIN

#include "quack_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/common/types/time.hpp"
#include "duckdb/common/types/timestamp.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

// OpenSSL linked through vcpkg
#include <openssl/opensslv.h>

#include <string>
#include <vector>
#include "../kdb_reader/kdbreader.hpp"
#include "../kdb_reader/read.hpp"
#include "../kdb_reader/utils.hpp"


#define EPOCH 10957  //1970.01.01~2000.01.01 days
#define EPOCH_MICRO 10957*24*60*60*1e6

#define DEBUG_MODE 0

namespace duckdb {

inline void QuackScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "Quack " + name.GetString() + " 🐥");
	});
}

inline void QuackOpenSSLVersionScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &name_vector = args.data[0];
	UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
		return StringVector::AddString(result, "Quack " + name.GetString() + ", my linked OpenSSL version is " +
		                                           OPENSSL_VERSION_TEXT);
	});
}


struct ReadKDBBindData : public TableFunctionData {
    bool finished = false;  
    std::vector<std::shared_ptr<KDBFileReader>> reader_vec;  
};



size_t fill_data(DataChunk& db_chunk, size_t col_idx, std::shared_ptr<KDBFileReader>& reader)
{
    size_t read_rows = 0;
    int dtype = reader->get_dtype();
    //size_t already_filled = reader->get_readed_size();
    switch(abs(dtype)) { 
        case 1: {
			std::vector<bool> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value::BOOLEAN(r_vec[row_idx]));
            }
			break;                
        }
        case 5: {
			std::vector<short> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value::SMALLINT(r_vec[row_idx]));
            }
			break;              
        }
        case 6: {
			std::vector<int> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value::INTEGER(r_vec[row_idx]));
            }
			break;              
        }
        case 7: {
			std::vector<long> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value::BIGINT(r_vec[row_idx]));
            }
			break;            
        }       
        case 8: {
			std::vector<float> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value::FLOAT(r_vec[row_idx]));
            }
			break;
        }
        case 9: {
			std::vector<double> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value::DOUBLE(r_vec[row_idx]));
            }              
			break;
        }
        case 4: {   //byte
            std::vector<char> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
                db_chunk.SetValue(col_idx, row_idx, Value(r_vec[row_idx]));  //as int
            }                  
            break;
        }
        case 10: {  //char
            std::vector<char> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
            char as_str[2] = {0};
            //std::cout<<"debug,case 10:"<<r_vec[0]<<std::endl;
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
                //db_chunk.SetValue(col_idx, row_idx, Value(r_vec[row_idx]));
                as_str[0] = r_vec[row_idx];
                db_chunk.SetValue(col_idx, row_idx, Value(as_str));  //as string
            }                  
            break;
        }
        case 11: {
			std::vector<std::string> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value(r_vec[row_idx]));
            }              
			break;
        }
        case 12: {
			std::vector<long> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;                
				db_chunk.SetValue(col_idx, row_idx, Value::TIMESTAMP(timestamp_t(EPOCH_MICRO+r_vec[row_idx]/1000)));
            }              
			break;            
        }         
        case 13: {
			std::vector<int> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
                int y = 2000+r_vec[row_idx]/12;
                int m = 1+r_vec[row_idx]%12;
				db_chunk.SetValue(col_idx, row_idx, Value::DATE(Date::FromDate(y,m,1)));
            }              
			break;            
        }  
        case 14: {
			std::vector<int> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
				db_chunk.SetValue(col_idx, row_idx, Value::DATE(duckdb::date_t(EPOCH+r_vec[row_idx])));
            }              
			break;            
        } 
        case 15: {
			std::vector<double> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;                
				db_chunk.SetValue(col_idx, row_idx, Value::TIMESTAMP(timestamp_t(EPOCH_MICRO+r_vec[row_idx]*86400000000)));
            }              
			break;            
        }           
        case 17: {
			std::vector<int> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
                //[t(a//60, a%60) for a in x]
                int h = r_vec[row_idx]/60;
                int m = r_vec[row_idx]%60;
				db_chunk.SetValue(col_idx, row_idx, Value::TIME(Time::FromTime(h,m,0,0)));
            }              
			break;            
        }
        case 18: {
			std::vector<int> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
                //[t(a//3600, a%3600//60, a%60) for a in x]
                int h = r_vec[row_idx]/3600;
                int m = r_vec[row_idx]%3600/60;
                int s = r_vec[row_idx]%60;
				db_chunk.SetValue(col_idx, row_idx, Value::TIME(Time::FromTime(h,m,s,0)));
            }              
			break;            
        }
        case 19: {
			std::vector<int> r_vec;
            size_t already_filled = reader->get_readed_size();
            read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
            if(read_rows<STANDARD_VECTOR_SIZE) {
                reader->set_finished();
            }
			for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
                //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
                //[t(a//3600000, a%3600000//60000, a%3600000%60000//1000, a%1000) for a in x]
                int h = r_vec[row_idx]/3600000;
                int m = r_vec[row_idx]%3600000/60000;
                int s = r_vec[row_idx]%3600000%60000/1000;
                int ms = r_vec[row_idx]%1000;
				db_chunk.SetValue(col_idx, row_idx, Value::TIME(Time::FromTime(h,m,s,ms)));
            }              
			break;            
        }              
    }

    if((dtype>=20)&&(dtype<=77)) {   //enum type
#if DEBUG_MODE 	    
        std::cout<<"info,fill_data enum"<<std::endl;
#endif	    
        std::vector<std::string> r_vec;
        size_t already_filled = reader->get_readed_size();
        read_rows = reader->read(already_filled,STANDARD_VECTOR_SIZE,r_vec); 
        if(read_rows<STANDARD_VECTOR_SIZE) {
            reader->set_finished();
        }
		for(size_t row_idx=0;row_idx<read_rows;row_idx++) {
            //std::cout<<"debug,row_idx:"<<row_idx<<std::endl;
			db_chunk.SetValue(col_idx, row_idx, Value(r_vec[row_idx]));
        }
    }

    // if(read_rows<STANDARD_VECTOR_SIZE) {
    //     reader->set_finished();
    // }
    return read_rows;
}



duckdb::LogicalType mapping_kdbdtype(int dtype) 
{
	duckdb::LogicalType r;
    switch(abs(dtype)) {
        case 0:
            std::cout<<"error,unknown datatype 0.TODO..."<<std::endl; 
            assert(0); 
			break;
        case 1:
			r = duckdb::LogicalType::BOOLEAN; 
			break;
        case 2:
            //TODO
            //readatom<bool>(fp, file_offset, 1); break;
            std::cout<<"error,unknown datatype 2/-2.TODO..."<<std::endl; 
            assert(0); 
            break;
        case 4:  //byte TODO
            r = duckdb::LogicalType::VARCHAR;
            break;
        case 5:
			r = duckdb::LogicalType::SMALLINT;
			break;
        case 6:
			r = duckdb::LogicalType::INTEGER;
            break;
        case 7:
            r = duckdb::LogicalType::BIGINT; 
			break;
        case 8:
            r = duckdb::LogicalType::FLOAT; 
			break;
        case 9:
            r = duckdb::LogicalType::DOUBLE;
			break;
        case 10: //char TODO
            r = duckdb::LogicalType::VARCHAR; 
			break;
        case 11:
            r = duckdb::LogicalType::VARCHAR; 
            break;
        case 12:
			r = duckdb::LogicalType::TIMESTAMP;   //microsecond(微秒)
            //read0<long>(dtype, fp, file_offset, byte_size); 
            //TODO, convert to timestamp

	// DUCKDB_API static Value TIMESTAMP(timestamp_t timestamp);
	// //! Create a timestamp_s Value from a specified value.
	// DUCKDB_API static Value TIMESTAMPSEC(timestamp_sec_t timestamp);
	// //! Create a timestamp_ms Value from a specified value.
	// DUCKDB_API static Value TIMESTAMPMS(timestamp_ms_t timestamp);
	// //! Create a timestamp_ns Value from a specified value.
	// DUCKDB_API static Value TIMESTAMPNS(timestamp_ns_t timestamp);
	// //! Create a timestamp_tz Value from a specified value.
	// DUCKDB_API static Value TIMESTAMPTZ(timestamp_tz_t timestamp);

            break;
        case 13:
			r = duckdb::LogicalType::DATE;
            break;                    
        case 14:
			r = duckdb::LogicalType::DATE;
            break; 
        case 15:
			r = duckdb::LogicalType::TIMESTAMP;
            //read0<double>(dtype, fp, file_offset, byte_size); 
            //TODO, convert to datetime
            break;             
        case 16:
			std::cout<<"error,unknown datatype 16/-16.TODO..."<<std::endl; 
            assert(0);

            //read0<long>(dtype, fp, file_offset, byte_size); 
            //TODO, convert to timespan

	// DUCKDB_API static Value INTERVAL(int32_t months, int32_t days, int64_t micros);
	// DUCKDB_API static Value INTERVAL(interval_t interval);

            break;             
        case 17:
			r = duckdb::LogicalType::TIME;
            //read0<int>(dtype, fp, file_offset, byte_size); 
            //TODO, convert to minute
            break; 
        case 18:
			r = duckdb::LogicalType::TIME;
            //read0<int>(dtype, fp, file_offset, byte_size); 
            //TODO, convert to second
            break; 
        case 19:
			r = duckdb::LogicalType::TIME;
            //read0<int>(dtype, fp, file_offset, byte_size); 
            //TODO, convert to time
            break; 
    }

    if((dtype>=20)&&(dtype<=77)) {  //enum type
        r = duckdb::LogicalType::VARCHAR;
    }

	return r;
}



std::shared_ptr<KDBFileReader>
 build_reader(const std::string& dir, const std::string& file_name, const std::string& sym_file_path, vector<LogicalType> &return_types, vector<string> &names)
{
    std::cout<<"info,build_reader(),reading file,dir:"<<dir<<",file:"<<file_name<<std::endl;
    std::string col_file = dir+"/"+file_name;
    if(!file_exist(col_file.c_str())) {
        std::cout<<"error,col file not exist:"<<col_file<<std::endl;
        return nullptr;
    }

    auto reader_ptr = std::make_shared<KDBFileReader>(col_file,sym_file_path);
    reader_ptr->read_meta();

    //std::cout<<"info,kdbfile_name:"<<file_name<<std::endl;
	names.push_back(file_name);
    reader_ptr->field_name_ = file_name;

    int dtype = reader_ptr->get_dtype();   
    //std::cout<<"info,dtype:"<<dtype<<std::endl; 
    duckdb::LogicalType tp = mapping_kdbdtype(dtype);
	return_types.push_back(tp); 
    return reader_ptr;
}

static unique_ptr<FunctionData> ReadKDBBind_list(ClientContext &context, TableFunctionBindInput &input,
    vector<LogicalType> &return_types, vector<string> &names) {

	auto bind_data = make_uniq<ReadKDBBindData>();
    // Extract the file path argument
    //auto &fs = FileSystem::GetFileSystem(context);

    string file_path = input.inputs[0].GetValue<string>();
	string sym_file_path = input.inputs[1].GetValue<string>(); 

    // for (auto &kv : input.named_parameters) 
    //     std::cout<<kv.first<<":"<<kv.second.GetValue<string>()<<std::endl;
    //std::cout<<"debug,named_parameters.count(file_path):"<<input.named_parameters.count("file_path")<<","<<input.named_parameters.count("sym_file_path")<<std::endl;

    // if(input.named_parameters.count("file_path") 
    // && input.named_parameters.count("sym_file_path")) {
    //     std::string file_path = input.named_parameters["file_path"].GetValue<std::string>();
    //     std::string sym_file_path = input.named_parameters["sym_file_path"].GetValue<std::string>();
        

        std::vector<std::string> dirs = split_string(file_path, "/");
        if(dirs.empty()) {
            std::cout<<"error,wrong path:"<<file_path<<std::endl;
            assert(dirs.size()>0);
        }
        
        std::string file_name = dirs.back();
        std::string root_dir = file_path.substr(0,file_path.size()-file_name.size()-1);
        std::shared_ptr<KDBFileReader> reader_ptr = build_reader(root_dir, file_name, sym_file_path, return_types, names);
        bind_data->reader_vec.push_back(reader_ptr);

    // }
    return std::move(bind_data);
}



static unique_ptr<FunctionData> ReadKDBBind_tbl(ClientContext &context, TableFunctionBindInput &input,
    vector<LogicalType> &return_types, vector<string> &names) {
    auto bind_data = make_uniq<ReadKDBBindData>();

    std::string root_dir = input.inputs[0].GetValue<string>();
    if(!is_directory(root_dir.c_str())) {
        std::cout<<"error,root_dir not exist:"<<root_dir<<std::endl;
        return std::move(bind_data);
    }
    
    std::string sym_file_path = root_dir+"/sym";

    std::string tbl_name = input.inputs[1].GetValue<string>();
    std::string tbl_dir;
    if(input.inputs.size()>3) {  //partitioned table
        std::string par_dir = input.inputs[3].GetValue<string>();
        tbl_dir = root_dir+"/"+par_dir+"/"+tbl_name; 
    } else {                    //splayed table
        tbl_dir = root_dir+"/"+tbl_name;
    }
    if(!is_directory(tbl_dir.c_str())) {
        std::cout<<"error,tbl_dir not exist:"<<tbl_dir<<std::endl;
        return std::move(bind_data);
    }

    std::string cols = input.inputs[2].GetValue<string>();
    std::vector<std::string> col_vec;
    cols = trim(cols);
    if(cols=="*" || cols=="") {
        std::string dotd_file_path = tbl_dir+"/.d";
        if(!file_exist(dotd_file_path.c_str())) {
            std::cout<<"error,.d file not exist:"<<dotd_file_path<<std::endl;
            return std::move(bind_data);
        }

        KDBFileReader reader(dotd_file_path, "");
        reader.read_meta();            
        reader.read<std::string>(0,10000,col_vec);
        std::cout<<"info,read .d file:"<<col_vec.size()<<std::endl; 
    } else {
        //user specified cols
        col_vec = split_string(cols, ",");
        for(size_t i=0;i<col_vec.size();i++)
            col_vec[i] = trim(col_vec[i]);
    }

    for(size_t i=0;i<col_vec.size();i++) {
        //std::cout<<"info,field:"<<col_vec[i]<<std::endl;
        auto reader_ptr = build_reader(tbl_dir, col_vec[i], sym_file_path, return_types, names);
        if(reader_ptr)
            bind_data->reader_vec.push_back(reader_ptr);
    }
        
	return std::move(bind_data);
}


static void ReadKDBFunction(ClientContext &context, TableFunctionInput &data, DataChunk &output) {
//     return ReadKDBFunction_impl<ReadKDBBindData>(context, data, output);
// }

// template<typename T>
// void ReadKDBFunction_impl(ClientContext &context, TableFunctionInput &data, DataChunk &output) {    
//    auto &bind_data = data.bind_data->CastNoConst<T>();
#if DEBUG_MODE
    std::cout<<"debug,ReadKDBFunction"<<std::endl;
#endif	
    auto &bind_data = data.bind_data->CastNoConst<ReadKDBBindData>();
    //auto &fs = FileSystem::GetFileSystem(context);
	//std::cout<<"info,output.ColumnCount():"<<output.ColumnCount()<<std::endl;
    
    // If already finished, signal end of data
    if (bind_data.finished) {
        output.SetCardinality(0);
        return;
    }

	//idx_t row_idx = 0;
    //idx_t max_rows = STANDARD_VECTOR_SIZE;  // Default DuckDB chunk size
    std::vector<size_t> read_rows_vec;
    for(size_t col_idx=0;col_idx<bind_data.reader_vec.size();col_idx++) {
        //std::cout<<"debug,ReadKDBFunction.col_idx:"<<col_idx<<std::endl;
        size_t read_rows = fill_data(output, col_idx, bind_data.reader_vec[col_idx]);
        read_rows_vec.push_back(read_rows);
    }

    //TODO:assert read_rows_vec has same value
    size_t read_rows = read_rows_vec.back();
    output.SetCardinality(read_rows);

    if(bind_data.reader_vec.back()->is_finished())
        bind_data.finished = true;
}	


// read_list(file_path,sym_file_path);
// read_splayed(root_dir,tbl_name,cols);
// read_partioned(root_dir,tbl_name,cols,par_dir);


static void LoadInternal(DatabaseInstance &instance) {
	// Register a scalar function
	auto quack_scalar_function = ScalarFunction("quack", {LogicalType::VARCHAR}, LogicalType::VARCHAR, QuackScalarFun);
	ExtensionUtil::RegisterFunction(instance, quack_scalar_function);

	// Register another scalar function
	auto quack_openssl_version_scalar_function = ScalarFunction("quack_openssl_version", {LogicalType::VARCHAR},
	                                                            LogicalType::VARCHAR, QuackOpenSSLVersionScalarFun);
	ExtensionUtil::RegisterFunction(instance, quack_openssl_version_scalar_function);


	auto read_list_func = TableFunction("read_list",
        {LogicalType::VARCHAR, LogicalType::VARCHAR},  ReadKDBFunction, ReadKDBBind_list);
    // read_list_func.named_parameters["file_path"] = LogicalType::VARCHAR;
    // read_list_func.named_parameters["sym_file_path"] = LogicalType::VARCHAR;
    ExtensionUtil::RegisterFunction(instance, read_list_func);

	auto read_splayed_func = TableFunction("read_splayed",
        {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR},  ReadKDBFunction, ReadKDBBind_tbl);
    // read_splayed_func.named_parameters["root_dir"] = LogicalType::VARCHAR;
    // read_splayed_func.named_parameters["tbl_name"] = LogicalType::VARCHAR;
    // read_splayed_func.named_parameters["cols"] = LogicalType::VARCHAR;
    ExtensionUtil::RegisterFunction(instance, read_splayed_func);

	auto read_partitioned_func = TableFunction("read_partitioned",
        {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR},  ReadKDBFunction, ReadKDBBind_tbl);
    // read_partitioned_func.named_parameters["root_dir"] = LogicalType::VARCHAR;
    // read_partitioned_func.named_parameters["tbl_name"] = LogicalType::VARCHAR;
    // read_partitioned_func.named_parameters["cols"] = LogicalType::VARCHAR;
    // read_partitioned_func.named_parameters["par_dir"] = LogicalType::VARCHAR;
    ExtensionUtil::RegisterFunction(instance, read_partitioned_func);

}

void QuackExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string QuackExtension::Name() {
    return "quack";
    //return "kdb";
}

std::string QuackExtension::Version() const {
#ifdef EXT_VERSION_QUACK
	return EXT_VERSION_QUACK;
#else
	return "";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void quack_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::QuackExtension>();
}

DUCKDB_EXTENSION_API const char *quack_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
