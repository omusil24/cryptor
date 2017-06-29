#include "FileSystem.h"
#include <assert.h>
#include <iostream>
#include <random>
#include <dirent.h>
#include <string.h>

#include "../Colors/Colors.h"
#include "../Crypto/Crypto.h"
#include "../ConsoleOutput/ConsoleOutput.h"
#include "../Functions.h"

FileSystem::FileSystem() : settings(nullptr)
{
}

FileSystem::~FileSystem()
{
    clear();
}

void FileSystem::clear()
{
    for (auto i = AllFiles.begin(); i != AllFiles.end(); i++)
        delete i->second;
    AllFiles.clear();

    for (auto i = exploredDirs.begin(); i != exploredDirs.end(); i++)
        delete i->second;
    exploredDirs.clear();
}

const unordered_map<string, FileDirEntity*>& FileSystem::FindAllFiles(const Settings& s)
{
    settings = &s;

    //clear all current data, if any
    clear();

    //print warning
    if (s.CheckForUnEncryptedFiles || s.find_encrypted)
    { //we only want to find files that do not have ,,encrypted" extension
        ConsoleOutput::print("It is not possible to check whether file is encrypted or not, application checks for expected file extension only.", ConsoleOutput::WARNING);
    }

    //get the files and dirs from command line
    auto fptr = s.GetFilesOrDirsToEncryptDecrypt();

    string extension = Crypto::ENCRYPTED_EXTENSION_;
    EXTENSION_MODE extension_mode = EXTENSION_MODE::OFF;

    if (s.CheckForUnEncryptedFiles)
        extension_mode = EXTENSION_MODE::EXCLUDE;
    else if (s.find_encrypted)
        extension_mode = EXTENSION_MODE::INCLUDE_ONLY;

    //iterate all provided arguments (files, dirs), not absolute paths
    for (auto it = fptr->begin(); it != fptr->end(); it++)
    {
        string filepath = *it;
        struct stat filedir_struct;
        if (!Functions::GetFileStat(filepath, filedir_struct))
            return AllFiles;

        if (Functions::isdir(filedir_struct))
        {// traverse the directory. Absolute paths are always returned for files
            //here no check is done for return value
            GetFilesInDirectory_rec(filepath, filedir_struct, s.recursive, s.verbose, extension, extension_mode);
        }
        else
        { //is not directory                       
            //get the absolute path for file, if exists
            string absolute_path;
            if (Functions::getFileDirAbsolutePath(filepath.c_str(), absolute_path))
            {
                if (Functions::matches_file_extension(absolute_path, extension, extension_mode))
                    insertFile(absolute_path, filedir_struct);
            }
            else
                ConsoleOutput::print("'" + filepath + "': no such file.", ConsoleOutput::ERROR);
        }
    }

    if (s.CheckForUnEncryptedFiles || s.find_encrypted)
    {
        if (AllFiles.size() == 0)
        {
            if (s.CheckForUnEncryptedFiles)
                ConsoleOutput::print("No unencrypted files found.", ConsoleOutput::OK);
            else //we wanted to find all encrypted files
                ConsoleOutput::print("No encrypted files found.", ConsoleOutput::OK);
        }
        else if (s.CheckForUnEncryptedFiles)
            cout << "Apparently unencrypted files, total:" << Colors::YELLOW << "[" << AllFiles.size() << "]" << Colors::RESET << endl;
        else
            cout << "Apparently encrypted files, total:" << Colors::YELLOW << "[" << AllFiles.size() << "]" << Colors::RESET << endl;
    }
    return AllFiles;
}

const unordered_map<string, FileDirEntity*>& FileSystem::GetFileList() const
{
    return AllFiles;
}

inline bool FileSystem::GetFilesInDirectory_rec(const string& d, const struct stat& dir_stat_data, bool recursive, bool verbose,
        const string& extension, EXTENSION_MODE m)
{
    if (!Functions::isdir(dir_stat_data))
    {
        ConsoleOutput::print("'" + d + "' : not a directory.", ConsoleOutput::ERROR, " ERROR ");
        return false;
    }

    string abspathdir;
    if (!Functions::getFileDirAbsolutePath(d.c_str(), abspathdir))
    {
        ConsoleOutput::print("'" + d + "' : no such directory.", ConsoleOutput::ERROR, " ERROR ");
        return false;
    }
    else if (Functions::isSLink(dir_stat_data))
    {
        ConsoleOutput::print("Refusing to traverse symlink dir '" + abspathdir + "'", ConsoleOutput::WARNING);
        return false;
    }

    if (!insertExploredDir(abspathdir, dir_stat_data))
        return true; //explored already

    if (verbose)
        cout << "Exploring directory '" << abspathdir << "'" << endl;

    bool status = true;

    DIR *dir;
    struct dirent *ent;
    dir = opendir(abspathdir.c_str());
    if (dir)
    {
        /* get all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            string filepath_w_dir = abspathdir + "/" + string(ent->d_name);

            struct stat Entity_statdata;
            if (!Functions::GetFileStat(filepath_w_dir, Entity_statdata))
            {
                closedir(dir);
                return false;
            }

            bool is_dir = Functions::isdir(Entity_statdata);
            if (recursive && is_dir)
            {
                if (!GetFilesInDirectory_rec(filepath_w_dir, Entity_statdata, recursive, verbose, move(extension), m))
                    status = false;
            }
            if (is_dir)
                continue;

            if (Functions::matches_file_extension(filepath_w_dir, extension, m))
                insertFile(filepath_w_dir, Entity_statdata);
        }
        closedir(dir);
    }
    else
    { /* could not open directory */
        ConsoleOutput::print("Can not open directory: '" + d + "'", ConsoleOutput::ERROR);
        return false;
    }

    return status;
}

inline bool FileSystem::insertExploredDir(const string& abs_path, const struct stat& s)
{
    FileDirEntity* FDE = new FileDirEntity(abs_path, s);
    bool res = exploredDirs.insert(make_pair(abs_path, FDE)).second;

    if (!res)
        delete FDE;
    return res;
}

inline bool FileSystem::insertFile(const string& abs_path, const struct stat& s)
{
    if (!(settings->CheckForUnEncryptedFiles || settings->find_encrypted) && Functions::isSLink(s))
    {
        if (settings->encrypt)
            ConsoleOutput::print("'" + abs_path + "' : is a symlink. Refusing to encrypt.", ConsoleOutput::WARNING);
        else
            ConsoleOutput::print("'" + abs_path + "' : is a symlink. Refusing to decrypt.", ConsoleOutput::WARNING);
        return false;
    }

    FileDirEntity* FDE = new FileDirEntity(abs_path, s);
    bool res = AllFiles.insert(make_pair(abs_path, FDE)).second;

    if (!res)
        delete FDE;
    else if (settings->CheckForUnEncryptedFiles || settings->find_encrypted)
    { //we only want to find files that do not have ,,encrypted" extension        
        cout << abs_path << endl;
    }

    return res;
}