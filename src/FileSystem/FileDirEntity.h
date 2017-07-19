/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   FileDirEntity.h
 * Author: root
 *
 * Created on June 23, 2017, 12:29 PM
 */

#ifndef FILEDIRENTITY_H
#define FILEDIRENTITY_H

#include <sys/stat.h>
#include <string>

using namespace std;

struct FileDirEntity {
    /**
     * 
     * @param abs_path
     * @param file_stat Must be already containing valid, initialized data
     */
    FileDirEntity(const string& abs_path, const struct stat& file_stat);
    FileDirEntity(string&& abs_path, const struct stat& file_stat);
    FileDirEntity(const string& abs_path);
    FileDirEntity(string&& abs_path);
    FileDirEntity(const FileDirEntity& a);
    ~FileDirEntity();

    const string& getAbsPath() const;
    ino_t getInode() const;
    size_t getSize() const;
    const string& getSymlinkPath() const;
    bool isDir() const;
private:
    bool init_getSymlinkPath();
public:
    bool isRegFile() const;
    bool isSymLink() const;

    bool ok() const;
    FileDirEntity& operator=(const FileDirEntity& a) = delete;

private:
    string abs_path_private;
    bool ok_private;
    struct stat stat_struct;
    string symlink_path;
};

#endif /* FILEDIRENTITY_H */

