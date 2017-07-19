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
    enum class STAT_RES_ENUM
    {
        FILE_DIR_DOES_NOT_EXIST,
        NO_PERMISSIONS,
        OK,
        UNKNOWN_ERR                
    };
    static string combinePaths(const string& a, const string& b);  
    static bool ExistsFile(const string& s, bool& FatalError);
    static string getExtension(const string& path);
    /**
     * 
     * @param p
     * @param path returned Absolute path for a file or directory
     * @return Whether getting the absolute path succeeded or not
     */
    static bool getFileDirAbsolutePath(const char* p, string& path);
    static string GetFileNameWithExtension(const string& entirePath);
    /**
     * @brief Returns file size using the point to the opened file.
     * The position within file stays unchanged by default
     * @param ptr pointer to opened file. Must be valid, or segfault will occur, if it is nullptr, 0 size is returned
     * @param return_position_within_file_back_to_original True by default, if true, the position within file stays unchanged,
     * else, position within file is changed to end.
     * @return 0 if pointer is NULL, else, if no error during seek, returns size of file.
     */
    static bool GetFileSize(FILE* ptr, size_t& fsize, bool return_position_within_file_back_to_original = true);
    static STAT_RES_ENUM GetFileStat(const string& abs_path, struct stat& struc, bool supressOutput = false);
    /**
     * It returns parent directory. For example, being given
     * /etc/var/file
     * it returns
     * /etc/var
     * It is advised to use CombinePaths() to avoid problems with concatenating.
     * For /file
     * it returns /
     */
    static string GetFileParentDir(const string& Path);
    static uint32_t GetRandomNumber(uint32_t min, uint32_t max = numeric_limits<uint32_t>::max());
    static bool isdir(const string& path);
    static bool isdir(const struct stat& file_stat);
    static bool isSLink(const struct stat& file_stat);
    
    static bool matches_file_extension(const string& abs_path, const string& extension, FileSystem::EXTENSION_MODE m);
    static string removeExtension(const string& path);
};

#endif /* FUNCTIONS_H */

