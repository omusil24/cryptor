#include "FileDirEntity.h"

#include <unistd.h>

#include "../Functions.h"
#include "../ConsoleOutput/ConsoleOutput.h"

FileDirEntity::FileDirEntity(const string& abs_path, const struct stat& file_stat) : abs_path_private(abs_path), ok_private(true), stat_struct(file_stat)
{
    ok_private = init_getSymlinkPath();
}

FileDirEntity::FileDirEntity(string&& abs_path, const struct stat& file_stat) : abs_path_private(move(abs_path)), ok_private(true), stat_struct(file_stat)
{
    ok_private = init_getSymlinkPath();
}

FileDirEntity::FileDirEntity(const string& abs_path) : abs_path_private(abs_path)
{
    ok_private = (Functions::GetFileStat(abs_path_private, stat_struct) == Functions::STAT_RES_ENUM::OK);
    if(ok_private)
        ok_private = init_getSymlinkPath();
}
FileDirEntity::FileDirEntity(string&& abs_path) : abs_path_private(move(abs_path))
{
    ok_private = (Functions::GetFileStat(abs_path_private, stat_struct) == Functions::STAT_RES_ENUM::OK);
    if(ok_private)
        ok_private = init_getSymlinkPath();
}
FileDirEntity::FileDirEntity(const FileDirEntity& a) : abs_path_private(a.abs_path_private),
    ok_private(a.ok_private), stat_struct(a.stat_struct), symlink_path(a.symlink_path)
{
    
}
FileDirEntity::~FileDirEntity()
{
}

const string& FileDirEntity::getAbsPath() const
{
    return abs_path_private;
}
ino_t FileDirEntity::getInode() const
{
    return stat_struct.st_ino;
}
size_t FileDirEntity::getSize() const
{
    return stat_struct.st_size;
}    
const string& FileDirEntity::getSymlinkPath() const
{
    if(!isSymLink())
    {
        ConsoleOutput::print("Trying to get symlink path for file that is not symlink", ConsoleOutput::CODE_ERROR);
        return getAbsPath();
    }
    return symlink_path;
}
bool FileDirEntity::isDir() const
{
    return S_ISDIR(stat_struct.st_mode);
}
bool FileDirEntity::init_getSymlinkPath()
{
    if(!isSymLink())
        return true;
    
    size_t MAX = 2048;
    char* buffer = new char[MAX];
    int wrote = readlink(abs_path_private.c_str(), buffer, MAX - 1);
    if(wrote == -1)
    {
        ConsoleOutput::print("readlink failed", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }
    if(wrote != (int)MAX - 1)
        buffer[wrote] = '\0';
    else
    {
        ConsoleOutput::print("readlink warning: Path inside symlink is too long to handle", ConsoleOutput::WARNING);
        buffer[MAX - 1] = '\0';
        
    }
    //get absolute path from that
    if(!Functions::getFileDirAbsolutePath(buffer, symlink_path))
    {
        ConsoleOutput::print("Symlink file is broken and pointing to non existent file. Can not get full path of the referenced file. The symlink content path will be encrypted as it is.", ConsoleOutput::ERROR);
        symlink_path = string(buffer);
        delete [] buffer;
        return true;
    }
    
    delete [] buffer;
    return true;
}
bool FileDirEntity::isRegFile() const
{
    return S_ISREG(stat_struct.st_mode);
}
bool FileDirEntity::isSymLink() const
{
    return S_ISLNK(stat_struct.st_mode);
}
bool FileDirEntity::ok() const
{
    return ok_private;
}