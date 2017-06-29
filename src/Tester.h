/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Tester.h
 * Author: root
 *
 * Created on June 14, 2017, 4:27 PM
 */

#ifndef TESTER_H
#define TESTER_H
#include <cinttypes>
#include <limits>
#include <string>

#include "FileSystem/FileDirEntity.h"
#include "Settings/Settings.h"

using namespace std;

struct Tester
{
    Tester(const string&& runningdir, const Settings& s);
    ~Tester();    
    
private:
    FileDirEntity Createfile(const string& runningdir, char*& buffer, uint32_t MaxFileSizeInBytes) const;

    inline string GetUniqueFileName() const;

public:
    /**
     * @brief runs the tests
     */
    bool run(uint16_t numberOfFilesToGenerate, uint32_t MaxFileSizeInBytes, uint32_t MinFileSizeInBytes = 0) const;

private:    
    bool CreateAndTestFile(uint32_t FileSizeInBytes) const;

    const string runningdir;
    
    const Settings* settings;
public:
    bool verbose;
};


#endif /* TESTER_H */

