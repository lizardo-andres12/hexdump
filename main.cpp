#include <array>
#include <climits>
#include <cstdio>
#include <filesystem>
#include <bitset>
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

    { "-E", false },
    { "-EBCDIC", false },

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

std::unique_ptr<std::string> parse_cmd(int argc, char **argv)
{
    std::unique_ptr<std::string> err = std::make_unique<std::string>("IO");
    std::string parsed_cmd;
    int count = 0;

    // options should always come first in command
    while (**(++argv) == '-')
    {
        const std::string opt_type = *argv;

        if (!validate_opt(opt_type))
            return err;

        if (OPT_REQUIRE_ARG[opt_type])
        {
            // using buffer to only write after passing all error checks
            const std::string opt_arg = *(++argv);
            std::string buffer;
            char sign = opt_arg[0];
            bool is_negative = false;

            if (sign == '-')
            {
                buffer += sign;
                is_negative = true;
            }

            // must skip character that holds sign, can take advantage
            // that is_negative can be cast to int and will set start
            // index to one if true (number is negative)
            for (int i = is_negative; i < opt_arg.size(); i++)
            {
                char cast_val = opt_arg[i];

                // 48-57 is the ascii character value of the numbers 0-9
                if (cast_val < 48 || cast_val > 57)
                    return err;

                buffer += cast_val;
            }

            parsed_cmd += (opt_type + buffer);
            count += 2;
        }
        else
        {
            // error existed where an option that takes no arguments exited loop
            // early, fixed by peeking at first char of next option and checking
            // equality to ascii value of '-'
            if (**(argv + 1) != 45)
                return err;
            parsed_cmd += opt_type;
            count++;
        }
    }

    // if no file or too many files were provided
    if (argc == count || argc + 2 < count)
        return err;

    for (int i = 0; i < argc - count; i++)
        parsed_cmd += *(argv++);

    return std::make_unique<std::string>(parsed_cmd);
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

bool is_vaild_file(std::ifstream &file)
{
    if (!file.is_open())
    {
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
    // ---- Change 16 to be argument accepted ----
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
    const std::string opt = "12";
    std::cout << validate_opt(opt);
    if (argc == 1) 
    {
        std::cerr << "No file path specified" << '\n';
        return INVALID_OP;
    }

    std::cout << *parse_cmd(argc - 1, argv);
    return 0;
    // must cast PATH into std::fs::path to use the exists method
    const std::string PATH(*(argv + 1));
    if (!std::filesystem::exists(static_cast<std::filesystem::path>(PATH))) 
    {
        std::cerr << "No file " << PATH << " in given directory" << '\n';
        return INPUT_FILE_ERROR;
    }

    // decode in this context means to turn binary into hex
    if (is_task_decode(PATH))
    {
        if (decode_file(PATH) == INPUT_FILE_ERROR)
            return INPUT_FILE_ERROR;
    }
    else
        if (encode_file(PATH) == INPUT_FILE_ERROR)
            return INPUT_FILE_ERROR;

    return OK;
}
