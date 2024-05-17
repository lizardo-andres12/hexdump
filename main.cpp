#include "cmd_parser.hpp"
#include "file_utils.hpp"
#include "error_codes.h"
#include <iostream>


int main(int argc, char **argv)
{
    if (argc == 1)
    {
        std::cerr << "No file path specified" << '\n';
        return PARSE_ERROR;
    }

    const CLA CMD_ARGS = *parse_cmd(argc, argv);
    if (CMD_ARGS.input_file == "err")
    {
        std::cerr << "Command is invalid" << '\n';
        return PARSE_ERROR;
    }

    std::map<std::string, int> opts_with_args = *validate_args(CMD_ARGS);
    if (opts_with_args["err"] == PARSE_ERROR)
    {
        std::cerr << "One or more option arguments were invalid" << '\n';
        return INVALID_OP;
    }

    int validation = validate_file(CMD_ARGS);
    if (validation == INPUT_FILE_ERROR)
    {
        std::cerr << "No file \"" << CMD_ARGS.input_file << "\" exists in directory" << '\n';
        return INPUT_FILE_ERROR;
    }

    if (opts_with_args["-b"] != -1)
        decode_file(CMD_ARGS.input_file);

    return OK;
}

