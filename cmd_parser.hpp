#ifndef CMD_PARSER_HPP
#define CMD_PARSER_HPP

#include <map>
#include <memory>
#include <string>


const int MAX_INT = 2147483647;

typedef struct CommandLineArgs
{
    std::map<std::string, std::string> opts_and_args;
    std::string input_file;
    std::string output_file;
} CLA;


bool validate_opt(const std::string &opt);
int string_to_int(const std::string &str);
int validate_file(const CLA &CMD_ARGS);
std::unique_ptr<std::map<std::string, int>> validate_args(const CLA &CMD_ARGS);
std::unique_ptr<CLA> parse_cmd(int argc, char **argv);

#endif
