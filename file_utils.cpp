#include "file_utils.hpp"
#include "constants.hpp"
#include "error_codes.h"
#include <bitset>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <memory>


bool _is_file_open(std::ifstream &file)
{
    if (!file.is_open())
    {
        file.close();
        return false;
    }

    return true;
}

int dump(std::map<std::string, int> opts_with_args, std::ifstream &file, std::ostream &output_target)
{
    // opening in binary mode to not convert any special characters
    // buffer is 16 because standard file offset is 16 bits
    // buffer of type char will cast binary values into ascii values
    // ---- Change 16 to be -c arg ----
    if (!_is_file_open(file))
        return INPUT_FILE_ERROR;

    if (opts_with_args["-b"] == -1 || opts_with_args["-bits"] == -1)
        encode_file(file, output_target);
    else
        decode_file(file, output_target);

    return OK;
}

int validate_file(const CLA &CMD_ARGS)
{
    // input file is guaranteed by parse_cmd()
    const std::filesystem::path INPUT_PATH(CMD_ARGS.input_file);

    if (!std::filesystem::exists(INPUT_PATH))
        return INPUT_FILE_ERROR;

    return OK;
}

std::unique_ptr<std::string> _dec_to_hex(int val)
{
    if (val == 0)
        return std::make_unique<std::string>("0");

    std::string output;
    while (val > 0)
    {
        int hex_val = val & 0xf;
        output = DEC_TO_HEX[hex_val] + output;
        val >>= 4;
    }

    return std::make_unique<std::string>(output);
}

std::unique_ptr<std::string> _get_ascii_conversion(char *buffer)
{
    std::string output;
    for (size_t i = 0; i < 16; i++)
    {
        if (buffer[i] < 33 || buffer[i] > 126)
            output += '.';
        else
            output += buffer[i];
    }

    return std::make_unique<std::string>(output);
}

std::unique_ptr<std::string> _get_decoded_row(char *buffer)
{
    std::string output;

    // ---- Change 16 to be offset value ----
    for (int i = 0; i < 16; i++) 
    {
        int byte_val = buffer[i];

        if (byte_val < 0)
            byte_val += 256;

        std::string hex = *_dec_to_hex(byte_val);
        if (hex.size() == 1)
            hex = '0' + hex;

        output += hex;

        // ---- Change 15 to offset value - 1 ----
        if (i % 2 == 1 && i != 15)
            output += ' ';
    }

    return std::make_unique<std::string>(output);
}

void decode_file(std::ifstream &file, std::ostream &output_target)
{
    char buffer[16];
    int rows = 0;

    while (!file.eof())
    {
        output_target << std::setfill('0') << std::setw(8);
        output_target << *_dec_to_hex(rows++ * 16) << ": ";

        // must use signed char buffer to use file.read()
        file.read(buffer, 16);
        output_target << *_get_decoded_row(buffer) << "  ";
        output_target << *_get_ascii_conversion(buffer);
        output_target << '\n';

        // flush output buffer and zero out input buffer to avoid overflow
        std::flush(output_target);
        std::fill(buffer, buffer + sizeof(buffer), 0);
    }
}

void encode_file(std::ifstream &file, std::ostream &output_target)
{
    char ascii[8];
    char buffer;
    int count = 0;
    int row = 0;

    while (file >> std::noskipws >> buffer)
    {
        // ---- Change 8 to -c arg ----
        if (count % 8 == 0 && count != 0)
        {
            output_target << ' ' << *_get_ascii_conversion(ascii);
            output_target << '\n';
        }

        if (count % 8 == 0)
        {
            output_target << std::setfill('0') << std::setw(8);
            output_target << *_dec_to_hex(row++ * 8) << ": ";
        }

        ascii[count % 8] = buffer;

        std::bitset<8> binary(buffer);
        output_target << binary << ' ';

        buffer = 0;
        std::flush(output_target);
        count++;
    }

    output_target << '\n';
}

