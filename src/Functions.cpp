#include "Functions.h"
#include <assert.h>
#include <iostream>
#include <random>
#include <dirent.h>

#include "ConsoleOutput/ConsoleOutput.h"

string Functions::combinePaths(const string& a, const string& b)
{
    if (a.length() == 0)
        return b;

    if (a.back() != '/')
        return a + "/" + b;
    else
        return a + b;
}
bool Functions::ExistsFile(const string& s, bool& FatalError)
{
    FatalError = false;
    
    struct stat stat_;
    auto res = GetFileStat(s, stat_, true);
    if(res == STAT_RES_ENUM::FILE_DIR_DOES_NOT_EXIST)
        return false;
    else if(res == STAT_RES_ENUM::OK)
        return true;
    else
        FatalError = true;
    return false;
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

string Functions::GetFileNameWithExtension(const string& entirePath)
{
    if(entirePath.length() == 0)
        return entirePath;
    
    char* buff = new char[entirePath.length() + 1];
    size_t buff_len = 0;
    size_t indexer = 0;
    bool found = false;
    for(size_t i = entirePath.length() - 1;;i--)
    {
        if(entirePath[i] == '/')
        {
            buff[indexer] = '\0';
            found = true;
            buff_len = indexer;
            break;
        }
        else
            buff[indexer] = entirePath[i];
        indexer++;
        if(i == 0)
            break;
    }
        
    if(!found)
    {
        delete [] buff;
        return entirePath;
    }
    //now the name is reversed, reverse it again
    string retur;
    for(size_t i = buff_len - 1; ; i--)
    {
        retur += buff[i];
        if(i == 0)
            break;
    }
    delete [] buff;
    return retur;
}

bool Functions::isdir(const struct stat& file_stat)
{
    return S_ISDIR(file_stat.st_mode);
}

bool Functions::isdir(const string& path)
{
    struct stat b;
    if (GetFileStat(path, b) != Functions::STAT_RES_ENUM::OK)
        return false;

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
    if (return_position_within_file_back_to_original)
        position = ftell(ptr);

    //get filesize 
    if (fseek(ptr, 0, SEEK_END) != 0)
    {
        cerr << "fseek failed." << endl;
        return false;
    }
    fsize = ftell(ptr);

    if (return_position_within_file_back_to_original)
    {
        if (fseek(ptr, position, SEEK_SET) != 0)
        {
            cerr << "fseek failed." << endl;
            return false;
        }
    }

    return true;
}

Functions::STAT_RES_ENUM Functions::GetFileStat(const string& abs_path, struct stat& struc, bool supressOutput)
{
    if (lstat(abs_path.c_str(), &struc) != 0)
    {
        if (errno == ENOENT)
        {
            //  doesn't exist
            if(!supressOutput)
                ConsoleOutput::print(abs_path + ": file or dir does not exist.", ConsoleOutput::ERROR);
            return STAT_RES_ENUM::FILE_DIR_DOES_NOT_EXIST;
        }
        else if (errno == EACCES)
        {
            // we don't have permission to know if 
            //  the path/file exists.. impossible to tell
            ConsoleOutput::print(abs_path + ": permission denied. Can not tell if file/dir really exists.", ConsoleOutput::ERROR);
            return STAT_RES_ENUM::NO_PERMISSIONS;
        }
        else
        {
            //general error handling
            ConsoleOutput::print("lstat failed", ConsoleOutput::FUNCTION_ERROR);
            return STAT_RES_ENUM::UNKNOWN_ERR;
        }
    }
    return STAT_RES_ENUM::OK;
}
string Functions::GetFileParentDir(const string& Path)
{
    string result;
    uint16_t LastSlash_position = 0;
    for(uint16_t i = 0; i < Path.length(); i++)
    {        
        if(Path[i] == '/')
            LastSlash_position = i;
        result += Path[i]; 
    }
    if(LastSlash_position == 0)
        return "/";
    
    return result.substr(0, LastSlash_position);
}
uint32_t Functions::GetRandomNumber(uint32_t min, uint32_t max)
{
    if (min > max)
        assert(false);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<uint32_t> distribution(min, max);
    return (uint32_t) distribution(gen);
}

bool Functions::matches_file_extension(const string& abs_path, const string& extension, FileSystem::EXTENSION_MODE extension_mode)
{
    if (extension_mode == FileSystem::EXTENSION_MODE::OFF)
        return true;

    bool include = (extension_mode == FileSystem::EXTENSION_MODE::INCLUDE_ONLY);

    if (include)
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