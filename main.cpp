#include <array>
#include <climits>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
// #include <vector>

// used for quick conversion from int to string
const std::array<std::string, 16> DEC_TO_HEX = {"0", "1", "2", "3", "4", "5",
    "6", "7", "8", "9", "a", "b",
    "c", "d", "e", "f"};

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

bool is_task_decode(const std::string &PATH) 
{
    // to see whether the file should be translated from binary
    // to hex or hex to binary I used the file {filename} linux command
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
            hex += " ";
    }

    // return the pointer for efficiency when passing to ascii converter
    std::unique_ptr<std::string> hex_ptr = std::make_unique<std::string>(hex);
    return hex_ptr;
}

int decode_file(const std::string &PATH) 
{
    // opening in binary mode to not convert any special characters
    // buffer is 16 because standard file offset is 16 bits
    // ---- Change 16 to be argument accepted ----
    std::ifstream file(PATH, std::ios::binary);
    char buffer[16];

    if (!file.is_open()) 
    {
        std::cerr << "Unable to open specified file" << '\n';
        file.close();
        return INPUT_FILE_ERROR;
    }

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

    // must cast PATH into std::fs::path to use the exists method
    const std::string PATH(*(argv + 1));
    if (!std::filesystem::exists(static_cast<std::filesystem::path>(PATH))) 
    {
        std::cerr << "No file " << PATH << " in given directory" << '\n';
        return INPUT_FILE_ERROR;
    }

    // decode in this context means to turn binary into hex
    if (is_task_decode(PATH))
        if (decode_file(PATH) == INPUT_FILE_ERROR)
            return INPUT_FILE_ERROR;
    return OK;
}
