#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string>
#include <limits>
#include <cinttypes>
#include <sys/stat.h>

#include "FileSystem/FileSystem.h"

using namespace std;

//stat   st_ino

struct Functions
{
    static string concat_paths(const string& a, const string& b);  
    static string getExtension(const string& path);
    /**
     * 
     * @param p
     * @param path returned Absolute path for a file or directory
     * @return Whether getting the absolute path succeeded or not
     */
    static bool getFileDirAbsolutePath(const char* p, string& path);
    /**
     * @brief Returns file size using the point to the opened file.
     * The position within file stays unchanged by default
     * @param ptr pointer to opened file. Must be valid, or segfault will occur, if it is nullptr, 0 size is returned
     * @param return_position_within_file_back_to_original True by default, if true, the position within file stays unchanged,
     * else, position within file is changed to end.
     * @return 0 if pointer is NULL, else, if no error during seek, returns size of file.
     */
    static bool GetFileSize(FILE* ptr, size_t& fsize, bool return_position_within_file_back_to_original = true);
    static bool GetFileStat(const string& abs_path, struct stat& struc);
    static uint32_t GetRandomNumber(uint32_t min, uint32_t max = numeric_limits<uint32_t>::max());
    static bool isdir(const string& path);
    static bool isdir(const struct stat& file_stat);
    static bool isSLink(const struct stat& file_stat);
    
    static bool matches_file_extension(const string& abs_path, const string& extension, FileSystem::EXTENSION_MODE m);
    static string removeExtension(const string& path);
};

#endif /* FUNCTIONS_H */

