#pragma once

#include <string>
#include <limits>
#include <cinttypes>
#include <sys/stat.h>
#include <unordered_map>
#include <sys/stat.h>

#include "FileDirEntity.h"
#include "../Settings/Settings.h"

using namespace std;

//stat   st_ino

struct FileSystem
{
    enum class EXTENSION_MODE
    {
        INCLUDE_ONLY, /**< If we want to find only files matching the extension given */
        EXCLUDE, /**< If we want to find only files matching everything BUT the extension given */
        OFF /**< If we want to ignore any extensuion filter */
    };
    
    FileSystem();
    FileSystem(const FileSystem& a) = delete;
    ~FileSystem();
    
private:
    void clear();
    
public:
    const unordered_map<string, FileDirEntity*>& FindAllFiles(const Settings& s);
    const unordered_map<string, FileDirEntity*>& GetFileList()const;
    
private:
   
    /**
     * @brief do not call this, this is called from GetFilesInDirectory. Call GetFilesInDirectory only
     * @param dir Directory to start search in
     * @param list The returned list of files. Contains absolute paths only, and they are distinct
     * @param exploredDirs 
     * @param recursive Whether function should be recursive
     * @param verbose Whether additional info shall be printed to the terminal during search
     * @param extension_all_excet
     * @return Whether all succeeded or there was an error
     */    
    inline bool GetFilesInDirectory_rec(const string& dir, 
            const struct stat& dir_stat_data,
            bool recursive, 
            bool verbose, 
            const string& extension, EXTENSION_MODE m);
    
    inline bool insertExploredDir(const string& abs_path, const struct stat& s);
    inline bool insertFile(const string& abs_path, const struct stat& s);
    
    
    unordered_map<string, FileDirEntity*> AllFiles;
    
private:
    unordered_map<string, FileDirEntity*> exploredDirs;
    const Settings* settings;
};
