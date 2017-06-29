#include "Functions.h"
#include <assert.h>
#include <iostream>
#include <random>
#include <dirent.h>

#include "ConsoleOutput/ConsoleOutput.h"

string Functions::concat_paths(const string& a, const string& b)
{
    if (a.length() == 0)
        return b;

    if (a.back() != '/')
        return a + "/" + b;
    else
        return a + b;
}

string Functions::getExtension(const string& path)
{
    size_t length = path.length();
    if (length == 0)
        return "";

    string extension = "";
    for (int i = length - 1;; i--)
    {
        if (path[i] == '.')
            break;
        extension = path[i] + extension;

        if (i == 0)
            break;
    }
    return extension;
}

bool Functions::getFileDirAbsolutePath(const char* p, string& path) 
{
    char actualpath [PATH_MAX + 1];
    char *ptr;
    ptr = realpath(p, actualpath);

    if (ptr)
        path = string(ptr);

    return ptr;
}

bool Functions::isdir(const struct stat& file_stat)
{
    return S_ISDIR(file_stat.st_mode);
}
bool Functions::isdir(const string& path)
{
    struct stat b;
    GetFileStat(path, b);
    return isdir(b);
}
bool Functions::isSLink(const struct stat& file_stat)
{
    return S_ISLNK(file_stat.st_mode);
}
bool Functions::GetFileSize(FILE* ptr, size_t& fsize, bool return_position_within_file_back_to_original)
{
    if (ptr == nullptr)
    {
        fsize = 0;
        return true;
    }

    size_t position = 0;
    if(return_position_within_file_back_to_original)
         position = ftell(ptr);

    //get filesize 
    if (fseek(ptr, 0, SEEK_END) != 0)
    {
        cerr << "fseek failed." << endl;
        return false;
    }
    fsize = ftell(ptr);
    
    if(return_position_within_file_back_to_original)
    {
        if (fseek(ptr, position, SEEK_SET) != 0)
        {
            cerr << "fseek failed." << endl;
            return false;
        }
    }

    return true;
}
bool Functions::GetFileStat(const string& abs_path, struct stat& struc)
{
    if (lstat(abs_path.c_str(), &struc) != 0)
    {
        ConsoleOutput::print("lstat failed", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }
    return true;
}
uint32_t Functions::GetRandomNumber(uint32_t min, uint32_t max)
{
    if(min > max)
        assert(false);
    
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<uint32_t> distribution(min, max);
    return (uint32_t) distribution(gen);
}
bool Functions::matches_file_extension(const string& abs_path, const string& extension, FileSystem::EXTENSION_MODE extension_mode)
{
    if(extension_mode == FileSystem::EXTENSION_MODE::OFF)
        return true;
    
    bool include = (extension_mode == FileSystem::EXTENSION_MODE::INCLUDE_ONLY);
    
    if(include)
        return (getExtension(abs_path) == extension);
    else
        return (getExtension(abs_path) != extension);
    return false;
}    
string Functions::removeExtension(const string& path)
{
    if (path.length() == 0)
        return "";

    string ext = getExtension(path);

    if (ext.length() == path.length())
        return path;

    if (ext.length() == 0)
        return path.substr(0, path.length() - 1);
    ;

    return path.substr(0, path.length() - ext.length() - 1);
}