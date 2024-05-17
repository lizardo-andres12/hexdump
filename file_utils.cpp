#include "file_utils.hpp"
#include "constants.hpp"
#include "error_codes.h"
#include <array>
#include <bitset>
#include <cstdio>
#include <iostream>


bool is_file_open(std::ifstream &file)
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

    if (!is_file_open(file))
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

    if (!is_file_open(file))
        return INPUT_FILE_ERROR;

    while (!file.eof()) 
    {
        file.read(buffer, 16);
        std::unique_ptr<std::string> hex = _convert_bytes_to_hex(buffer);
        std::cout << *hex << '\n';

        // flush output buffer and zero out
        // input buffer to avoid overflow
        std::flush(std::cout);
        std::fill(buffer, buffer + sizeof(buffer), 0);
    }

    file.close();
    return OK;
}

std::unique_ptr<std::string> _convert_bytes_to_hex(char *buffer)
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
