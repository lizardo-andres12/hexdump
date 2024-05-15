#include <array>
#include <bitset>
#include <climits>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

// used for quick conversion from int to string
const std::array<std::string, 16> DEC_TO_HEX = {"0", "1", "2", "3", "4", "5",
    "6", "7", "8", "9", "a", "b",
    "c", "d", "e", "f"};

std::unordered_map<std::string, bool> OPT_REQUIRE_ARG = {
    { "-a", false },
    { "-autoskip", false },

    { "-b", false },
    { "-bits", false },

    { "-c", true },
    { "-cols", true },

    { "-g", true },
    { "-groupsize", true },

    { "-h", false },
    { "-help", false },

    { "-l", true },
    { "-len", true },


    { "-p", false },
    { "-ps", false },
    { "-postscript", false },
    { "-plain", false },

    { "-s", true },
    { "-seek", true },

    { "-u", false },

    { "-v", false }
};

enum ERROR_CODES {
    OK = 1,
    PARSE_ERROR,
    INPUT_FILE_ERROR,
    OUTPUT_FILE_ERROR,
    INVALID_SEEK,
    INVALID_OP = -1,
};

/*  traditional xxd command requires filepath input    ----- hexdump is from
binary to other formats ----- typedef std::vector<std::filesystem::path>
pathvec;

void readdir(std::string dir, pathvec* vec)
{
    for (const auto &entry : std::filesystem::directory_iterator(dir))
    {
         if (entry.is_directory()) vec->push_back(entry.path());
    }
}


std::filesystem::path fsearch(const std::string fname, const std::string dir)
{
    pathvec subdirs;
    std::filesystem::path dirpath(dir);
    std::filesystem::path fnameindir = dirpath / fname;

    std::clog << "Searching in directory " << dirpath.string() << '\n';

    if (std::filesystem::exists(fnameindir)) return fnameindir;

    readdir(dir, &subdirs);
    for (const auto &subdir : subdirs)
    {
        fnameindir = subdir / fname;
    }
    return dirpath;
}

bool is_task_decode(const std::string &PATH) 
{
    // to see whether the file should be translated from binary to
    // hex or hex to binary I used the file {filename} linux command
    // to get file description. If the file is not binary, the output
    // of the commmand would not constain the word "ASCII"
    const char ASCII[] = {'A', 'S', 'C', 'I', 'I'};
    const std::string BASHCMD = "file " + PATH;
    const std::string OUTPUT = *get_cmd_output(PATH, BASHCMD);

    for (int i = 0, counter = 0; i < OUTPUT.size(); i++) 
    {
        if (OUTPUT[i] == ASCII[counter]) 
        {
            counter++;
            if (counter == 5)
                return false;
        }
        else
            counter = 0;
    }

    return true;
}
*/

std::unique_ptr<std::string> get_cmd_output(const std::string &PATH,
                           const std::string &BASHCMD)
{
    // used std::array instead of c-style array for methods
    // could possibly error if the ASCII part of description is
    // over 128 characters in description
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(BASHCMD.c_str(), "r"),
                                                  pclose);
    fgets(buffer.data(), buffer.size(), pipe.get());

    std::unique_ptr<std::string> output_ptr = std::make_unique<std::string>(buffer.data());
    return output_ptr;
}

std::unique_ptr<std::string> convert_bytes_to_hex(char *buffer) 
{
    std::string hex;

    // ---- Change 16 to be offset value ----
    for ( int i = 0; i < 16; i++) 
    {
        int byte_val = buffer[i];

        // char to int conversion causes two's complement error
        // changing byteval to unsigned causes segmentation fault
        // manual conversion if byte_val is negative
        if (byte_val < 0)
            byte_val += 256;

        hex += (DEC_TO_HEX[byte_val / 16] + DEC_TO_HEX[byte_val % 16]);

        // last space is omitted to reduce ascii translation complexity
        // ---- Change 15 to offset value - 1 ----
        if (i % 2 == 1 && i != 15)
            hex += ' ';
    }

    // return the pointer for efficiency when passing to ascii converter
    std::unique_ptr<std::string> hex_ptr = std::make_unique<std::string>(hex);
    return hex_ptr;
}

bool validate_opt(const std::string &opt)
{
    auto it = OPT_REQUIRE_ARG.find(opt);
    if (it == OPT_REQUIRE_ARG.end())
        return false;

    return true;
}

std::unique_ptr<std::unordered_map<std::string, int>> handle_opts(const std::vector<std::string> &COMMAND_ARGS, std::vector<int> &opt_arg_values, int optc)
{
    std::unordered_map<std::string, int> opts_to_args{};
    for (int i = 0, val_i = 0; i < optc; i++)
    {
        std::string opt = COMMAND_ARGS[i];
        if (OPT_REQUIRE_ARG[opt])
            opts_to_args[opt] = opt_arg_values[val_i++];
        else
            // cannot set to 0 since 0 is produced when opt isn't a
            // key in the map
            opts_to_args[opt] = -1;
    }

    return std::make_unique<std::unordered_map<std::string, int>>(opts_to_args);
}

std::unique_ptr<std::vector<std::string>> parse_cmd(int argc, char **argv, int &optc, int &filec, std::vector<int> &opt_arg_values)
{
    std::unique_ptr<std::vector<std::string>> err = std::make_unique<std::vector<std::string>>();
    err->push_back("err");

    std::vector<std::string> parsed_cmd{};
    int parsed_strings = 0;

    // options should always come first in command
    while (**(++argv) == '-')
    {
        const std::string opt_type = *argv;
        optc++;

        if (!validate_opt(opt_type))
            return err;

        if (OPT_REQUIRE_ARG[opt_type])
        {
            const std::string opt_arg = *(++argv);
            char sign = opt_arg[0];
            char cast_val;
            bool is_negative = false;

            if (sign == '-')
                is_negative = true;

            // must skip character that holds sign, can take advantage
            // that is_negative can be cast to int and will set start
            // index to one if true (number is negative)
            short opt_argv = 0;
            for (int i = is_negative; i < opt_arg.size(); i++)
            {
                cast_val = opt_arg[i];

                // 48-57 is the ascii character value of the numbers 0-9
                if (cast_val < 48 || cast_val > 57)
                    return err;

                // convert from ascii to decimal
                cast_val -= 48;
                short converter = std::pow(10, opt_arg.size() - i - 1);
                opt_argv += (cast_val * converter);
            }

            if (is_negative)
                opt_argv *= -1;

            opt_arg_values.push_back(opt_argv);
            parsed_cmd.push_back(opt_type);
            parsed_strings += 2;
        }
        else
        {
            parsed_cmd.push_back(opt_type);
            parsed_strings++;
        }
    }

    // if no file was given or loop exited too early
    // best way to catch unneeded argument errors is to
    // let the while loop break and check how many opts and
    // args were parsed
    if (argc == parsed_strings || argc > parsed_strings + 2)
        return err;

    for (int i = 0; i < argc - parsed_strings; i++)
    {
        parsed_cmd.push_back(*(argv++));
        filec++;
    }

    return std::make_unique<std::vector<std::string>>(parsed_cmd);
}

bool is_vaild_file(std::ifstream &file)
{
    if (!file.is_open())
    {
        // ---- Refactor error logging to main function using INPUT_FILE_ERROR ----
        std::cerr << "Unable to open specified file" << '\n';
        file.close();
        return false;
    }

    return true;
}

int encode_file(const std::string &PATH)
{
    // file opened in both encode and decode functions
    // to guarantee close when variable goes out of scope
    std::ifstream file(PATH);
    char buffer;
    char count = 0;

    if (!is_vaild_file(file))
        return INPUT_FILE_ERROR;

    while (file >> std::noskipws >> buffer)
    {
        count++;
        std::bitset<8> binary(buffer);
        std::cout << binary << ' ';

        // ---- Change 5 to -c arg ----
        if (count % 5 == 0)
        {
            std::cout << '\n';
        }
    }

    file.close();
    return OK;
}

int decode_file(const std::string &PATH) 
{
    // opening in binary mode to not convert any special characters
    // buffer is 16 because standard file offset is 16 bits
    // buffer of type char will cast binary values into ascii values
    // ---- Change 16 to be -c arg ----
    std::ifstream file(PATH, std::ios::binary);
    char buffer[16];

    if (!is_vaild_file(file))
        return INPUT_FILE_ERROR;

    while (!file.eof()) 
    {
        file.read(buffer, 16);
        std::unique_ptr<std::string> hex = convert_bytes_to_hex(buffer);
        std::cout << *hex << '\n';

        // flush output buffer and zero out
        // input buffer to avoid overflow
        std::flush(std::cout);
        std::fill(buffer, buffer + sizeof(buffer), 0);
    }

    file.close();
    return OK;
}

int main(int argc, char **argv) 
{
    if (argc == 1)
    {
        std::cerr << "No file path specified" << '\n';
        return INVALID_OP;
    }

    int optc = 0, filec = 0;
    std::vector<int> opt_arg_values{};
    const std::vector<std::string> COMMAND_ARGS = *parse_cmd(argc - 1, argv, optc, filec, opt_arg_values);
    if (COMMAND_ARGS[0] == "err")
    {
        std::cerr << "Invalid command" << '\n';
        return INVALID_OP;
    }

    // all command validation done in parse_cmd() function
    std::unordered_map<std::string, int> opts_with_args = *handle_opts(COMMAND_ARGS, opt_arg_values, optc);

    // must cast PATH into std::fs::path to use the exists method

    const std::string PATH(argv[argc - filec]);
    if (!std::filesystem::exists(static_cast<std::filesystem::path>(PATH))) 
    {
        std::cerr << "No file " << PATH << " in given directory" << '\n';
        return INPUT_FILE_ERROR;
    }

    // decode in this context means to turn binary into hex
    if (opts_with_args["-b"] != -1)
        decode_file(PATH);

    return OK;
}
