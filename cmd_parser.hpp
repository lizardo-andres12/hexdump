#ifndef CMD_PARSER_HPP
#define CMD_PARSER_HPP

#include "structs.h"
#include <memory>
#include <string>


bool validate_opt(const std::string &opt);
int string_to_int(const std::string &str);
std::unique_ptr<std::map<std::string, int>> validate_args(const CLA &CMD_ARGS);
std::unique_ptr<CLA> parse_cmd(int argc, char **argv);

#endif
