#ifndef ERROR_CODES_H
#define ERROR_CODES_H

enum ERROR_CODES {
    OK = 1,
    PARSE_ERROR,
    INPUT_FILE_ERROR,
    OUTPUT_FILE_ERROR,
    INVALID_SEEK,
    INVALID_OP = -1,
};

#endif