#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include "structs.h"
#include <fstream>
#include <map>
#include <memory>
#include <iostream>
#include <string>


bool is_file_open(std::ifstream &file);
int dump(std::map<std::string, int> opts_with_args, std::ifstream &file, std::ostream &output_target);
int validate_file(const CLA &CMD_ARGS);
std::unique_ptr<std::string> _dec_to_hex(int val);
std::unique_ptr<std::string> _get_decoded_row(char *buffer);
void decode_file(std::ifstream &file, std::ostream &output_target);
void encode_file(std::ifstream &file, std::ostream &output_target);

#endif
