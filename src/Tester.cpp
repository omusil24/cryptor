#include "Tester.h"

#include "Crypto/Crypto.h"

#include <assert.h>
#include <iostream>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fstream>
#include <limits>

#include "ConsoleOutput/ConsoleOutput.h"
#include "Functions.h"
#include "MemoryWiper.h"
#include "Crypto/CryptResult/CryptResult.h"

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

Tester::Tester(const string&& runningdir_, const Settings& s) : runningdir(runningdir_), settings(&s), verbose(false)
{
}

Tester::~Tester()
{
}

FileDirEntity Tester::Createfile(const string& runningdir, char*& buffer, uint32_t MaxFileSizeInBytes) const
{
    string fname = GetUniqueFileName();

    ofstream os;
    os.open(fname, ios::out | ios::binary | ios::trunc);

    delete [] buffer;
    buffer = new char[MaxFileSizeInBytes];

    uint32_t buffer_pos = 0;
    while (buffer_pos < MaxFileSizeInBytes)
    {
        buffer[buffer_pos] = Functions::GetRandomNumber(0);

        os.write(&buffer[buffer_pos], 1);
        buffer_pos++;
    }

    os.close();

    FileDirEntity FDE(move(fname));
    return FDE;
}

bool Tester::CreateAndTestFile(uint32_t FileSizeInBytes) const
{
    char* buffer = nullptr;

    if (verbose)
        ConsoleOutput::print("Creating file [" + to_string(FileSizeInBytes) + " Bytes]", ConsoleOutput::OK);

    FileDirEntity FDE = Createfile(runningdir, buffer, FileSizeInBytes);
    if (verbose)
        ConsoleOutput::print("File '" + FDE.getAbsPath() + "' created", ConsoleOutput::OK);

    Crypto C(*settings);
    string key = "abc";
    C.SetEncryptionKey(key);
    MemoryWiper::ClearVariable(&key);

    CryptResult res;
    C.encrypt(FDE, &res);
    C.WaitForFinish();

    if (res.getResult() != Crypto::CRYPT_RESULT::OK)
    {
        ConsoleOutput::print("Encrypt failed", ConsoleOutput::ERROR);
        return false;
    }

    FileDirEntity Enc_FDE(res.final_filename);
    C.decrypt(Enc_FDE, &res);
    C.WaitForFinish();

    if (res.getResult() != Crypto::CRYPT_RESULT::OK)
    {
        ConsoleOutput::print("Decrypt failed", ConsoleOutput::ERROR);
        return false;
    }

    FILE* dec = nullptr;
    dec = fopen(res.final_filename.c_str(), "rb");
    if (!dec)
    {
        ConsoleOutput::print("Can not open file'" + res.final_filename + "'", ConsoleOutput::ERROR);
        return false;
    }
    char* readbuffer = new char[FileSizeInBytes];
    char* buffer_position = buffer;

    bool testsuccess = true;

    int red = 0;
    while (true)
    {
        red = fread(readbuffer, 1, 1024, dec);
        if (red <= 0)
            break;

        if (memcmp(buffer_position, readbuffer, red) != 0)
        {
            ConsoleOutput::print("The original and decrypted files do not match. Test failed.", ConsoleOutput::ERROR);

            testsuccess = false;
            break;
        }
        buffer_position += red;
    }
    fclose(dec);

    //check if original input file name matches the decrypted one. Wich it should
    if (FDE.getAbsPath() == res.final_filename)
        ConsoleOutput::print("The input and decrypted filenames match.", ConsoleOutput::OK);
    else
    {
        ConsoleOutput::print("The input and decrypted filenames do not match.", ConsoleOutput::ERROR);
        testsuccess = false;
    }
    if (testsuccess)
    {
        if (remove(res.final_filename.c_str()) != 0)
        {
            testsuccess = false;
            ConsoleOutput::print("Error deleting test file", ConsoleOutput::ERROR);
        }
    }
    else
    {
        FILE* in_err;
        string in_err_filename = FDE.getAbsPath() + "_input_error";
        in_err = fopen(in_err_filename.c_str(), "wb");
        fwrite(buffer, 1, FileSizeInBytes, in_err);
        fclose(in_err);

        ConsoleOutput::print("Creating file with the orininal input on wich test failed ... ", ConsoleOutput::OK);
        ConsoleOutput::print("File " + in_err_filename + " created ... ", ConsoleOutput::OK);

        ConsoleOutput::print("Test failed on file '" + FDE.getAbsPath() + "' (that is file after decryption). File will not be deleted to allow further testing.", ConsoleOutput::ERROR);
    }
    if (verbose && testsuccess)
        ConsoleOutput::print("Decryption encryption OK", ConsoleOutput::OK);


    delete [] buffer;
    delete [] readbuffer;
    return testsuccess;
}

string Tester::GetUniqueFileName() const
{

    uint32_t i = Functions::GetRandomNumber(1);
    string name = Functions::combinePaths(runningdir, to_string(i) + ".test");

    uint16_t iterations = 0;
    while (true)
    {
        FILE* fd = nullptr;
        fd = fopen(name.c_str(), "rw");
        if (fd == nullptr)
            break;

        fclose(fd);


        i = rand() % numeric_limits<uint16_t>::max() + 1;
        name = Functions::combinePaths(runningdir, to_string(i) + ".test");
        iterations++;

        if (iterations > 1500)
        {
            ConsoleOutput::print("Failed to find unique name for file", ConsoleOutput::ERROR);
            assert(false);
        }
    }

    return name;
}

bool Tester::run(uint16_t numberOfFilesToGenerate, uint32_t MaxFileSizeInBytes, uint32_t MinFileSizeInBytes) const
{
    ConsoleOutput::print("Running test:\n[Files to generate: " + to_string(numberOfFilesToGenerate) + "]\n" +
            "[Max file size: " + to_string(MaxFileSizeInBytes) + " Bytes ]\n" +
            "[Min file size: " + to_string(MinFileSizeInBytes) + " Bytes ]\n", ConsoleOutput::OK);


    uint16_t tests_succeded = 0;
    uint32_t LargestFile = 0;
    for (uint16_t i = 0; i < numberOfFilesToGenerate; i++)
    {
        cout << "Test [" << i + 1 << " / " << numberOfFilesToGenerate << "]" << endl;
        uint32_t filesize = Functions::GetRandomNumber(MinFileSizeInBytes, MaxFileSizeInBytes);

        if (filesize > LargestFile)
            LargestFile = filesize;

        if (CreateAndTestFile(filesize))
            tests_succeded++;
    }
    cout << "Summary: " << endl;
    cout << "Tests succeeded: [" << tests_succeded << " / " << numberOfFilesToGenerate << "]" << endl;
    cout << "Largest file tested: [" << LargestFile << "] bytes" << endl;

    return (tests_succeded == numberOfFilesToGenerate);
}
