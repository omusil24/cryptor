#include "FileDirEntity.h"
#include "../Functions.h"

FileDirEntity::FileDirEntity(const string& abs_path, const struct stat& file_stat) : abs_path_private(abs_path), ok_private(true), stat_struct(file_stat)
{
}

FileDirEntity::FileDirEntity(string&& abs_path, const struct stat& file_stat) : abs_path_private(move(abs_path)), ok_private(true), stat_struct(file_stat)
{

}

FileDirEntity::FileDirEntity(string&& abs_path) : abs_path_private(move(abs_path))
{
    ok_private = Functions::GetFileStat(abs_path, stat_struct);
}
FileDirEntity::FileDirEntity(const FileDirEntity& a) : abs_path_private(a.abs_path_private),
    ok_private(a.ok_private), stat_struct(a.stat_struct)
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
bool FileDirEntity::isDir() const
{
    return S_ISDIR(stat_struct.st_mode);
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