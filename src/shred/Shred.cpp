#include "Shred.h"
//#include "BytesGenerator.h"

#include <fstream>
#include <iterator>
#include <iostream>

#include <sys/mman.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "BytesGenerator.h"
#include "../Functions.h"
#include "../ConsoleOutput/ConsoleOutput.h"

using namespace std;

Shred::Shred(uint8_t numberOfPasses_) :
numberOfPasses(numberOfPasses_),
verbose_private(false)
{
}

Shred::~Shred()
{
}

inline bool Shred::do_pass(int file_descriptor, PassType generator_type,
        uint64_t F_size) const
{
    if (file_descriptor < 0)
    {
        ConsoleOutput::print("Invalid file descriptor '" + to_string(file_descriptor) + "'", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }
    uint16_t BUF_SIZE = 700;

    void * buf = malloc(BUF_SIZE);
    memset(buf, 0, BUF_SIZE);
    ssize_t ret = 0;
    off_t shift = 0;
    while (1)
    {
        ret = write(file_descriptor, buf, ((F_size - shift > BUF_SIZE) ? BUF_SIZE : (F_size - shift)));

        if (ret <= 0)
            break;
        shift += ret;
    }

    bool status = true;

    free(buf);
    if (ret == -1)
    {
        ConsoleOutput::print("Failed to write data to file", ConsoleOutput::FUNCTION_ERROR);
        status = false;
    }

    return status;
}

void Shred::print(string&& what, bool m) const
{
    if (!verbose_private)
        return;

    if (m)
        cerr << "cryptor: shred: " << what << endl;
    else
        cout << "cryptor: shred: " << what << endl;

}

bool& Shred::verbose()
{
    return verbose_private;
}

bool Shred::WipeFile(const string& AbsolutePath)
{
    if (verbose_private)
        ConsoleOutput::print("[Shred] Wiping file started.", ConsoleOutput::OK);

    //open in read mode to get file size
    FILE* fin = nullptr;
    fin = fopen(AbsolutePath.c_str(), "rb");

    if (!fin)
    {
        ConsoleOutput::print("[Shred] " + AbsolutePath + ": No such file or directory or not a file.", ConsoleOutput::ERROR);
        return false;
    }

    bool res = true;

    //get filesize 
    size_t F_size = 0;
    if (!Functions::GetFileSize(fin, F_size, false))
        res = false;

    if (fclose(fin) != 0)
    {
        ConsoleOutput::print("[Shred] Getting file length failed: Failed to close the file.", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }

    if (!res)
        return false;

    //=========================================================================

    if (F_size > 0)
    {
        int fd = open(AbsolutePath.c_str(), O_WRONLY);

        //check again in case file no longer exists
        if (fd < 0)
        {
            ConsoleOutput::print("[Shred] " + AbsolutePath + ": No such file or not a file", ConsoleOutput::ERROR);
            return false;
        }

        for (uint8_t i = 0; i < numberOfPasses; i++)
        {
            if (lseek(fd, 0, SEEK_SET) < 0)
            {
                ConsoleOutput::print("[Shred][lseek] Failed to move to the beginning of file", ConsoleOutput::FUNCTION_ERROR);
                res = false;
                break;
            }
            if (!do_pass(fd, PassType::Predefined_Patterns, F_size))
            {
                res = false;
                ConsoleOutput::print("[Shred] Wiping file: pass #" + to_string(i) + " failed", ConsoleOutput::ERROR);
            }
            else if (verbose_private)
                ConsoleOutput::print("[Shred] Wiping file: pass #" + to_string(i) + " OK", ConsoleOutput::OK);
        }

        if (res)
        {//if still no error
            if (lseek(fd, 0, SEEK_SET) < 0)
            {
                ConsoleOutput::print("[Shred][lseek] Failed to move to the beginning of file", ConsoleOutput::FUNCTION_ERROR);
                res = false;
            }
            else if (!do_pass(fd, PassType::Zeros, F_size))
            {
                res = false;
                ConsoleOutput::print("[Shred] Wiping file: overwriting with zeros failed", ConsoleOutput::ERROR);
            }
        }
        if (close(fd) < 0)
        {
            ConsoleOutput::print("[Shred] Failed to close the file", ConsoleOutput::FUNCTION_ERROR);
            return false;
        }


        if (!res)
        { //there was some error so do not continue removing file
            return false;
        }
    }
    //remove the file finally
    if (remove(AbsolutePath.c_str()) != 0)
    {
        ConsoleOutput::print("[Shred] Error removing file", ConsoleOutput::FUNCTION_ERROR);
        res = false;
    }
    else if (verbose_private)
        ConsoleOutput::print("[Shred] File deleted", ConsoleOutput::OK);

    if (verbose_private)
        ConsoleOutput::print("[Shred] Wiping file ended", ConsoleOutput::OK);

    return res;
}