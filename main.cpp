#include <array>
#include <climits>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
// #include <vector>

const std::array<std::string, 16> DEC_TO_HEX = {"0", "1", "2", "3", "4", "5",
                                                "6", "7", "8", "9", "a", "b",
                                                "c", "d", "e", "f"};
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
std::string get_cmd_output(const std::string &PATH,
                           const std::string &BASHCMD) {
  std::array<char, 128> buffer;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(BASHCMD.c_str(), "r"),
                                                pclose);
  fgets(buffer.data(), buffer.size(), pipe.get());
  return buffer.data();
}

bool is_task_decode(const std::string &PATH) {
  const char ASCII[] = {'A', 'S', 'C', 'I', 'I'};
  const std::string BASHCMD = "file " + PATH;
  const std::string output = get_cmd_output(PATH, BASHCMD);

  for (int i = 0, counter = 0; i < output.size(); i++) {
    if (output[i] == ASCII[counter]) {
      counter++;
      if (counter == 5)
        return false;
    } else
      counter = 0;
  }

  return true;
}

std::string convert_bytes_to_hex(char *buffer) {
  std::string hex;

  for (int i = 0; i < 16; i++) {
    int byte_val = buffer[i];
    if (byte_val < 0)
      byte_val += 256;

    hex += (DEC_TO_HEX[byte_val / 16] + DEC_TO_HEX[byte_val % 16]);
    if (i % 2 == 1 && i != 15)
      hex += " ";
  }

  return hex;
}

void decode_file(const std::string &PATH) {
  std::ifstream file(PATH, std::ios::binary);
  char buffer[16];

  while (!file.eof()) {
    file.read(buffer, 16);
    std::cout << convert_bytes_to_hex(buffer) << '\n';
    std::flush(std::cout);
    std::fill(buffer, buffer + sizeof(buffer), 0);
  }
}

int main(int argc, char **argv) {
  if (argc == 1) {
    std::cerr << "No file path specified" << '\n';
    return 3;
  }

  const std::string PATH(*(argv + 1));
  if (!std::filesystem::exists(static_cast<std::filesystem::path>(PATH))) {
    std::cerr << "No file " << PATH << " in given directory" << '\n';
    return 3;
  }

  const std::ifstream FILE(PATH);
  if (!FILE.is_open()) {
    std::cerr << "Unable to open specified file" << '\n';
    return 3;
  }

  if (is_task_decode(PATH))
    decode_file(PATH);
  return 0;
}
