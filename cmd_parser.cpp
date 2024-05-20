#include "cmd_parser.hpp"
#include "constants.hpp"
#include "error_codes.h"
#include <cmath>
#include <memory>
#include <vector>


bool _validate_opt(const std::string &opt)
{
    auto it = OPT_REQUIRE_ARG.find(opt);
    if (it == OPT_REQUIRE_ARG.end())
        return false;

    return true;
}

int _string_to_int(const std::string &str)
{
    bool is_neg = false;
    if (str[0] == '-')
        is_neg = true;

    int intv = 0;
    for (size_t i = is_neg; i < str.size(); i++)
    {
        if (str[i] < 48 || str[i] > 57)
            return MAX_INT;

        intv += (str[i] - 48) * std::pow(10, str.size() - i - 1);
    }

    return intv;
}

std::unique_ptr<std::map<std::string, int>> validate_args(const CLA &CMD_ARGS)
{
    std::map<std::string, int> opts_and_args{};
    std::map<std::string, int> err{};
    err["err"] = PARSE_ERROR;

    for (auto &[opt, arg] : CMD_ARGS.opts_and_args)
    {
        if (OPT_REQUIRE_ARG.at(opt))
        {
            int argv = _string_to_int(arg);
            if (argv == MAX_INT)
                return std::make_unique<std::map<std::string, int>>(err);

            opts_and_args[opt] = argv;
        }
        else
            opts_and_args[opt] = -1;
    }

    return std::make_unique<std::map<std::string, int>>(opts_and_args);
}

std::unique_ptr<CLA> parse_cmd(int argc, char **argv)
{
    CLA cla;
    CLA err;
    err.input_file = "error";

    std::vector<std::string> args{};
    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);

    for (size_t i = 0; i < args.size(); i++)
    {
        if (args[i][0] == '-')
        {
            std::string option = args[i];
            if (!_validate_opt(option))
                return std::make_unique<CLA>(err);

            if (OPT_REQUIRE_ARG.at(option))
            {
                if (i + 1 < args.size())
                    cla.opts_and_args[option] = args[++i];
            }
            else
                cla.opts_and_args[option] = "";
        }
        else
        {
            if (cla.input_file.empty())
                cla.input_file = args[i];
            else if (cla.output_file.empty())
                cla.output_file = args[i];
            else
                return std::make_unique<CLA>(err);
        }
    }

    if (cla.input_file.empty())
        return std::make_unique<CLA>(err);

    return std::make_unique<CLA>(cla);
}
