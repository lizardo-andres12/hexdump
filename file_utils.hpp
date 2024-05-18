#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <fstream>
#include <memory>
#include <string>


bool is_file_open(std::ifstream &file);
int encode_file(const std::string &PATH);
int decode_file(const std::string &PATH);
std::unique_ptr<std::string> _dec_to_hex(int val);
std::unique_ptr<std::string> _get_decoded_row(char *buffer);

#endif
