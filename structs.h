#ifndef STRUCTS_H
#define STRUCTS_H


#include <map>
#include <string>


typedef struct CommandLineArgs
{
    std::map<std::string, std::string> opts_and_args;
    std::string input_file;
    std::string output_file;
} CLA;

#endif
