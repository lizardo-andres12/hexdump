#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <fstream>
#include <memory>
#include <string>


bool is_file_open(std::ifstream &file);
int encode_file(const std::string &PATH);
int decode_file(const std::string &PATH);
std::unique_ptr<std::string> _convert_bytes_to_hex(char *buffer);

#endif
