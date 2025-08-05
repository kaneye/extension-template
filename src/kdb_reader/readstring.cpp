#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For snprintf
#include "readstring.hpp"

// 定义文件 "a" 中偏移量的起始位置
#define FILE_A_OFFSET_START 0x1000


/*
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <base_file_path_A>\n", argv[0]);
        return 1;
    }

    const char* base_file_name = argv[1]; // 获取命令行参数作为文件A的路径
    int num_strings = 0;
    char** string_array = NULL;

    printf("Attempting to read strings from '%s' and '%s#'...\n\n", base_file_name, base_file_name);

    // 调用函数读取字符串数组
    string_array = readStringArray(base_file_name, &num_strings);

    if (string_array) {
        printf("Read %d strings:\n", num_strings);
        for (int i = 0; i < num_strings; ++i) {
            printf("[%d]: %s\n", i, string_array[i]);
        }
        // 释放内存
        freeStringArray(string_array, num_strings);
    } else {
        printf("Failed to read string array. Please ensure the files exist and are correctly formatted.\n");
    }

    return 0;
}
*/


/**
 * @brief 从指定文件读取字符串数组
 * @param base_file_name 文件 'A' 的名称（如 "data.bin"），函数会自动查找 "data.bin#"
 * @param num_strings 指向一个整数的指针，用于返回读取到的字符串数量
 * @return 成功时返回字符串指针数组的指针，失败时返回 NULL
 */
char** readStringArray(const char* base_file_name, int* num_strings) {
    FILE *file_a = NULL;
    FILE *file_a_hash = NULL;
    char** string_array = NULL;
    int current_string_count = 0;
    long file_a_size = 0;
    char file_a_hash_name[256]; // 假设文件名不会太长

    *num_strings = 0; // 初始化字符串数量

    // 构建 file_a_hash_name
    snprintf(file_a_hash_name, sizeof(file_a_hash_name), "%s#", base_file_name);

    // 打开文件 'a' (base_file_name)
    file_a = fopen(base_file_name, "rb");
    if (!file_a) {
        perror("Error opening base file");
        return NULL;
    }

    // 打开文件 'a#' (file_a_hash_name)
    file_a_hash = fopen(file_a_hash_name, "rb");
    if (!file_a_hash) {
        perror("Error opening hash file");
        fclose(file_a); // 确保已打开的文件被关闭
        return NULL;
    }

    // 获取文件 'a' 的大小，用于确定有多少个偏移量
    fseek(file_a, 0, SEEK_END);
    file_a_size = ftell(file_a);
    fseek(file_a, FILE_A_OFFSET_START, SEEK_SET); // 定位到偏移量起始位置

    // 计算有多少个8字节的偏移量组
    int max_offsets = (file_a_size - FILE_A_OFFSET_START) / 8;
    if (max_offsets < 0) {
        fprintf(stderr, "Error: FILE_A_OFFSET_START is beyond base file size or file is too small.\n");
        fclose(file_a);
        fclose(file_a_hash);
        return NULL;
    }

    // 预分配字符串数组的空间
    string_array = (char**)malloc(max_offsets * sizeof(char*));
    if (!string_array) {
        perror("Error allocating memory for string array");
        fclose(file_a);
        fclose(file_a_hash);
        return NULL;
    }

    unsigned char offset_bytes_buf[8]; // 用于读取7字节并补齐为8字节
    unsigned long long raw_offset_val; // 对应 Python 中的 struct.unpack('<Q', ...) 的结果
    long actual_data_offset_in_a_hash; // file_a_hash 中的实际数据偏移量
    char first_byte;
    unsigned char string_len_89;
    unsigned long long string_len_FB;
    char* current_string = NULL;

    // 循环读取偏移量并解析字符串
    int i;
    for (i = 0; i < max_offsets; ++i) {
        // 读取 7 字节的偏移量
        if (fread(offset_bytes_buf, 1, 7, file_a) != 7) {
            if (feof(file_a)) {
                // 如果是文件末尾，正常退出循环
                break;
            } else {
                perror("Error reading 7-byte offset from base file");
                freeStringArray(string_array, current_string_count);
                fclose(file_a);
                fclose(file_a_hash);
                return NULL;
            }
        }
        // 跳过剩余的 1 字节（因为是 8 字节一组）
        if (fseek(file_a, 1, SEEK_CUR) != 0) {
            perror("Error seeking 1 byte in base file");
            freeStringArray(string_array, current_string_count);
            fclose(file_a);
            fclose(file_a_hash);
            return NULL;
        }

        // 补齐第 8 个字节为 0，然后解析为 64 位无符号整数 (小端)
        offset_bytes_buf[7] = 0x00;
        memcpy(&raw_offset_val, offset_bytes_buf, 8);

        // 计算在文件 'a#' 中的实际数据偏移量
        actual_data_offset_in_a_hash = FILE_A_OFFSET_START + (long)raw_offset_val;


        // 定位到文件 'a#' 中的数据位置
        if (fseek(file_a_hash, actual_data_offset_in_a_hash, SEEK_SET) != 0) {
            perror("Error seeking in hash file");
            freeStringArray(string_array, current_string_count);
            fclose(file_a);
            fclose(file_a_hash);
            return NULL;
        }

        // 读取第一个字节，判断数据格式
        if (fread(&first_byte, 1, 1, file_a_hash) != 1) {
            // 如果读取失败，可能是文件过早结束或损坏
            perror("Error reading first byte from hash file");
            freeStringArray(string_array, current_string_count);
            fclose(file_a);
            fclose(file_a_hash);
            return NULL;
        }

        long string_length = 0;

        if (first_byte == (char)0x89) {
            // 格式 0x89: 第二个字节是长度 N
            if (fread(&string_len_89, 1, 1, file_a_hash) != 1) {
                perror("Error reading length (0x89 format) from hash file");
                freeStringArray(string_array, current_string_count);
                fclose(file_a);
                fclose(file_a_hash);
                return NULL;
            }
            string_length = (long)string_len_89;
        } else if (first_byte == (char)0xFB) {
            // 格式 0xFB: 从当前位置 (读取 first_byte 后) 跳过 7 字节到达第 8 字节
            // 然后读取 8 字节的长度
            if (fseek(file_a_hash, 7, SEEK_CUR) != 0) { // 跳过 7 字节
                perror("Error seeking 7 bytes in hash file (0xFB format)");
                freeStringArray(string_array, current_string_count);
                fclose(file_a);
                fclose(file_a_hash);
                return NULL;
            }

            if (fread(&string_len_FB, 8, 1, file_a_hash) != 1) {
                perror("Error reading length (0xFB format) from hash file");
                freeStringArray(string_array, current_string_count);
                fclose(file_a);
                fclose(file_a_hash);
                return NULL;
            }
            string_length = (long)string_len_FB;
        } else {
            fprintf(stderr, "Unknown string format (first byte: 0x%02X) at offset 0x%lX in hash file. Skipping.\n",
                    (unsigned char)first_byte, actual_data_offset_in_a_hash);
            continue; // 跳过当前这个无法解析的字符串
        }

        if (string_length <= 0) {
            fprintf(stderr, "Invalid string length (%ld) at offset 0x%lX in hash file. Skipping.\n",
                    string_length, actual_data_offset_in_a_hash);
            continue;
        }

        // 分配内存来存储字符串 (长度 + 1 用于 null 终止符)
        current_string = (char*)malloc(string_length + 1);
        if (!current_string) {
            perror("Error allocating memory for string");
            freeStringArray(string_array, current_string_count);
            fclose(file_a);
            fclose(file_a_hash);
            return NULL;
        }

        // 读取字符串内容
        if (fread(current_string, 1, string_length, file_a_hash) != string_length) {
            perror("Error reading string content from hash file");
            free(current_string); // 释放当前字符串的内存
            freeStringArray(string_array, current_string_count); // 释放之前所有字符串的内存
            fclose(file_a);
            fclose(file_a_hash);
            return NULL;
        }
        current_string[string_length] = '\0'; // 添加 null 终止符

        // 将读取到的字符串添加到数组中
        string_array[current_string_count++] = current_string;
    }

    // 调整数组大小到实际的字符串数量 (如果 max_offsets 远大于实际字符串数，可以节省内存)
    // 如果没有读取到任何字符串，并且 string_array 已经分配过，需要在这里释放
    char** final_string_array = NULL;
    if (current_string_count > 0) {
        final_string_array = (char**)realloc(string_array, current_string_count * sizeof(char*));
        if (!final_string_array) {
            perror("Error reallocating memory for final string array");
            // realloc 失败，string_array 仍然有效，可以选择不释放或报错
            // 这里选择保持 string_array 原样并返回，但会丢失一些内存优化
            *num_strings = current_string_count; // 仍然报告已读取的数量
            fclose(file_a);
            fclose(file_a_hash);
            return string_array;
        }
        string_array = final_string_array;
    } else {
        free(string_array); // 没有读取到字符串，释放初始分配的内存
        string_array = NULL;
    }


    *num_strings = current_string_count;

    fclose(file_a);
    fclose(file_a_hash);

    return string_array;
}

/**
 * @brief 释放字符串数组及其内部字符串的内存
 * @param string_array 要释放的字符串数组
 * @param num_strings 字符串数组中字符串的数量
 */
void freeStringArray(char** string_array, int num_strings) {
    if (string_array) {
	int i;
        for (i = 0; i < num_strings; ++i) {
            free(string_array[i]);
        }
        free(string_array);
    }
}