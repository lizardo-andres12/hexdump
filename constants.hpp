#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <array>
#include <map>
#include <string>

// used for quick conversion from int to string
const std::array<std::string, 16> DEC_TO_HEX = {"0", "1", "2", "3", "4", "5",
    "6", "7", "8", "9", "a", "b",
    "c", "d", "e", "f"};

const std::map<std::string, bool> OPT_REQUIRE_ARG = {
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

#endif
