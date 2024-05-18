#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <map>
#include <string>

// used for quick conversion from int to string
const char DEC_TO_HEX[] = "0123456789abcdef";

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
