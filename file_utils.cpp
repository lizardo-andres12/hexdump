#include "file_utils.hpp"
#include "constants.hpp"
#include "error_codes.h"
#include <bitset>
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

int encode_file(const std::string &PATH)
{
    // file opened in both encode and decode functions
    // to guarantee close when variable goes out of scope
    std::ifstream file(PATH);

    if (!_is_file_open(file))
        return INPUT_FILE_ERROR;

    char buffer;
    int count = 0;
    int row = 0;

    while (file >> std::noskipws >> buffer)
    {
        // ---- Change 8 to -c arg ----
        if (count % 8 == 0 && count != 0)
            std::cout << '\n';

        if (count % 8 == 0)
        {
            std::cout << std::setfill('0') << std::setw(8);
            std::cout << *_dec_to_hex(row++ * 8) << ": ";
        }

        std::bitset<8> binary(buffer);
        std::cout << binary << ' ';

        buffer = 0;
        std::flush(std::cout);
        count++;
    }

    std::cout << '\n';
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

    if (!_is_file_open(file))
        return INPUT_FILE_ERROR;

    char buffer[16];
    int rows = 0;

    while (!file.eof()) 
    {
        std::cout << std::setfill('0') << std::setw(8);
        std::cout << *_dec_to_hex(rows++ * 16) << ": ";

        file.read(buffer, 16);
        std::cout << *_get_decoded_row(buffer) << '\n';

        // flush output buffer and zero out input buffer to avoid overflow
        std::flush(std::cout);
        std::fill(buffer, buffer + sizeof(buffer), 0);
    }

    file.close();
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

std::unique_ptr<std::string> _get_decoded_row(char *buffer)
{
    std::string output;

    // ---- Change 16 to be offset value ----
    for (int i = 0; i < 16; i++) 
    {
        int byte_val = buffer[i];

        // char to int conversion causes two's complement error
        // changing byteval to unsigned causes segmentation fault
        // manual conversion if byte_val is negative
        if (byte_val < 0)
            byte_val += 256;

        std::string hex = *_dec_to_hex(byte_val);
        if (hex.size() == 1)
            hex = '0' + hex;

        output += hex;

        // last space is omitted to reduce ascii translation complexity
        // ---- Change 15 to offset value - 1 ----
        if (i % 2 == 1 && i != 15)
            output += ' ';
    }

    // return the pointer for efficiency when passing to ascii converter
    return std::make_unique<std::string>(output);
}
