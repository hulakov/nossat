#pragma once

#include <vector>
#include <cstdint>

class FileSystem final
{
  public:
    FileSystem();
    ~FileSystem();

    bool load_file(const char *path, std::vector<uint8_t> &buffer);
    size_t get_file_size(const char *path);
};