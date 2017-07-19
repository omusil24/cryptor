#include "Crypto.h"

#include <string>
#include <iostream>
#include <fstream>
#include <openssl/sha.h>
#include <thread>

#include "../Functions.h"
#include "../MemoryWiper.h"
#include "../shred/Shred.h"
#include "../ConsoleOutput/ConsoleOutput.h"
#include "../Settings/Settings.h"
#include "ThreadTask.h"
#include "CryptResult/CryptResult.h"

#include <assert.h>

#include <sstream>
#include <iomanip>


const string Crypto::ENCRYPTED_EXTENSION_ = "PH";
const size_t Crypto::FILE_SIZE_THREADS = 500000;

Crypto::Crypto(const Settings& s) :
init_ok_private(true), PasswordSet(false), settings_private(&s), threads(nullptr)
{
}

Crypto::~Crypto()
{
    //this deletes the threads
    WaitForFinish();

    /* Clean up */
    MemoryWiper::ClearVariable(&Password_hash);

    //for memory dumps, make it difficult to find anything
    //put here some other data
    settings_private = (Settings*)this;
    PasswordSet = false;
        
    EVP_cleanup();
    ERR_free_strings();
}
//=================================================

inline void Crypto::add_thread(ThreadTask* t)
{
    if (!threads)
        threads = new list<ThreadTask*>();
    threads->push_back(t);
}

void Crypto::aes_cryptFILE(const FileDirEntity& en, EVP_CIPHER_CTX* e, Crypto::ENRYPT_DECRYPT encrypt, CryptResult* result)
{
    string pathToFile = en.getAbsPath();

    FILE* in;
    in = fopen(pathToFile.c_str(), "rb");
    if (in == nullptr)
    {
        ConsoleOutput::print("Failed to open file.", ConsoleOutput::ERROR);
        result->setResult(CRYPT_RESULT::FILE_OPEN_FAILED);
        return;
    }

    //IF WE DECRYPT, try to read the head of encrypted file

    string ResultingFileName;
    if (encrypt == ENRYPT_DECRYPT::DE)
    {
        string original_file_name_DERYPTED_in_file;
        //if the original file name fas encrypted and is in file
        bool filename_changed;

        if (!readFileHead(in, result, filename_changed, original_file_name_DERYPTED_in_file))
        { //here pasword is not correct
            closeFile(in);
            return; //result is set and message is printed
        } //if pass is correct, now position in file is behind hash so we can continue reading....   

        if (filename_changed)
        {//combine the path with the newly decrypted file name
            string dir = Functions::GetFileParentDir(pathToFile);
            ResultingFileName = Functions::combinePaths(dir, original_file_name_DERYPTED_in_file);
        }
        else //just remove ENCRYTED file extension from name
            ResultingFileName = getResultingFileName(encrypt, pathToFile);
    }
    else
        ResultingFileName = getResultingFileName(encrypt, pathToFile);

    result->final_filename = ResultingFileName;

    bool fatal_functionError_Or_no_permissions;
    //check if the file exists already
    
    //---------------------------------------------------------------------------------------------------
    //make it thread safe
    //---------------------------------------------------------------------------------------------------
    mtx_file_create.lock();
    
    bool exists = Functions::ExistsFile(ResultingFileName, fatal_functionError_Or_no_permissions);
    if (exists && !fatal_functionError_Or_no_permissions)
    {//problem
        //find suitable name
        uint16_t i = 1;
        string tmp_name;
        while (true)
        {
            tmp_name = ResultingFileName + " (" + to_string(i) + ")";

            if (!Functions::ExistsFile(tmp_name, fatal_functionError_Or_no_permissions) || fatal_functionError_Or_no_permissions)
                break;

            if (i > 4000)
            {
                ConsoleOutput::print("Failed to find suitable non existing name for new file (processing file '" + pathToFile + "'.", ConsoleOutput::ERROR);
                result->setResult(CRYPT_RESULT::FILE_CREATE_FAILED);
                return;
            }
            i++;
        }

        ResultingFileName = tmp_name;

        if (encrypt == ENRYPT_DECRYPT::DE)
        {
            ConsoleOutput::print("Original file name was red, but such a file already exists. Altering name from '"
                    + result->final_filename + "' to '" + ResultingFileName + "'", ConsoleOutput::WARNING);
        }
        result->final_filename = ResultingFileName;
    }

    if (fatal_functionError_Or_no_permissions)
    {
        ConsoleOutput::print("Unrecoverable error. No permissions or function failure. Quit", ConsoleOutput::ERROR);
        result->setResult(CRYPT_RESULT::FILE_CREATE_FAILED);
        
        mtx_file_create.unlock();
        return;
    }
    
    FILE* fout = nullptr;
    fout = fopen(ResultingFileName.c_str(), "wb");
    
    //after file is created (if is created), no need for lock anymore
    mtx_file_create.unlock();
    
    if (!fout)
    {
        closeFile(in);
        ConsoleOutput::print("Failed to create a new file.", ConsoleOutput::ERROR);
        result->setResult(CRYPT_RESULT::FILE_CREATE_FAILED);
        return;
    }

    if (encrypt == ENRYPT_DECRYPT::ENC)
    {
        if (!writeFileHead(fout, result, pathToFile))
        {
            closeFile(in);
            closeFile(fout);
            return;
        }
    }

    //continue encrypting / decrypting in classic way if it is not symlink
    unsigned char* inbuf = new unsigned char[READ_CHUNK];
    unsigned char* outbuf = new unsigned char[READ_CHUNK + BLOCKSIZE];
    int out_len = 0;

    uint red = 0;
    while (true)
    {
        red = fread(inbuf, 1, READ_CHUNK, in);
        if (red <= 0) break;

        bool res;
        if (encrypt == ENRYPT_DECRYPT::ENC)
            res = EVP_EncryptUpdate(e, outbuf, &out_len, inbuf, red);
        else
            res = EVP_DecryptUpdate(e, outbuf, &out_len, inbuf, red);

        if (!res)
        {
            delete [] outbuf;
            delete [] inbuf;

            MemoryWiper::ClearVariable(&ResultingFileName);

            closeFile(fout);
            closeFile(in);

            result->setResult(CRYPT_RESULT::ENCRYPT_DECRYPT_FAILED);
            return;
        }
        //cout << "wrote " << out_len << " to file" << endl;

        if (fwrite(outbuf, 1, out_len, fout) != (size_t) out_len)
        {
            delete [] outbuf;
            delete [] inbuf;
            ConsoleOutput::print("[fwrite] Failed to write to file.", ConsoleOutput::FUNCTION_ERROR);

            closeFile(fout);
            closeFile(in);

            result->setResult(CRYPT_RESULT::FILE_WRITE_FAILED);
            return;
        }
    }

    int finallen = 0;
    if (encrypt == ENRYPT_DECRYPT::ENC)
        EVP_EncryptFinal_ex(e, outbuf, &finallen);
    else
        EVP_DecryptFinal_ex(e, outbuf, &finallen);

    if (fwrite(outbuf, 1, finallen, fout) != (size_t) finallen)
    {
        delete [] outbuf;
        delete [] inbuf;
        ConsoleOutput::print("[fwrite] Failed to write to file.", ConsoleOutput::FUNCTION_ERROR);
        closeFile(fout);
        closeFile(in);
        result->setResult(CRYPT_RESULT::FILE_WRITE_FAILED);
        return;
    }
    //cout << "wrote " << finallen << " to file" << endl;
    if (!closeFile(in))
    {
        delete [] outbuf;
        delete [] inbuf;
        result->setResult(CRYPT_RESULT::FILE_CLOSE_FAILED);
        return;
    }

    if (!closeFile(fout))
    {
        delete [] outbuf;
        delete [] inbuf;
        result->setResult(CRYPT_RESULT::FILE_CLOSE_FAILED);
        return;
    }
    else if (!settings_private->no_delete)
    {
        Shred s;
        s.WipeFile(pathToFile);
    }


    delete [] outbuf;
    delete [] inbuf;
    MemoryWiper::ClearVariable(&ResultingFileName);

    result->setResult(CRYPT_RESULT::OK);
}

bool Crypto::closeFile(FILE* f) const
{
    int res = fclose(f);
    if (res != 0)
        ConsoleOutput::print("[fclose] Failed to close file.", ConsoleOutput::FUNCTION_ERROR);

    return (res == 0);
}

EVP_CIPHER_CTX* Crypto::create_context(ENRYPT_DECRYPT e) const
{
    //password is the hash, so the size is fixed
    unsigned char *key_data = (unsigned char*) Password_hash.c_str();
    uint16_t key_data_len = Password_hash.length();

    unsigned char salt[8];
    salt[0] = key_data[5];
    salt[1] = key_data[7];
    salt[2] = key_data[1];
    salt[3] = key_data[5];
    salt[4] = key_data[3];
    salt[5] = key_data[2];
    salt[6] = key_data[2];
    salt[7] = key_data[0];


    int i, nrounds = 5;

    unsigned char key[32], iv[32];

    EVP_CIPHER_CTX* context = nullptr;
    context = EVP_CIPHER_CTX_new();

    bool status = true;
    /*
     * Gen key & IV for AES 256 CBC mode. A SHA1 digest is used to hash the supplied key material.
     * nrounds is the number of times the we hash the material. More rounds are more secure but
     * slower.
     */
    i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), (unsigned char *) &salt, key_data, key_data_len, nrounds, key, iv);
    if (i != 32)
    {
        ConsoleOutput::print("Key size is " + to_string(i) + " bits - should be 256 bits.", ConsoleOutput::CODE_ERROR);
        status = false;
    }
    else
    {
        if (e == ENRYPT_DECRYPT::ENC)
            EVP_EncryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv);
        else
            EVP_DecryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv);
    }
    MemoryWiper::ClearVariable(&nrounds);
    MemoryWiper::ClearVariable(&key[0], 32);
    MemoryWiper::ClearVariable(&iv[0], 16);
    MemoryWiper::ClearVariable(&salt[0], 8);

    if (!status)
    {
        EVP_CIPHER_CTX_free(context);
        return nullptr;
    }
    return context;
}

void Crypto::crypt_file(const FileDirEntity& ent, ENRYPT_DECRYPT e, CryptResult* result)
{
    if (!result)
    {
        ConsoleOutput::print("Logic error, result is null", ConsoleOutput::CODE_ERROR);
        result->setResult(CRYPT_RESULT::ENCRYPT_DECRYPT_FAILED);
        return;
    }
    //checks for other stuff and administration
    if (!PasswordSet)
    {
        ConsoleOutput::print("Encrypt: Error, no password is set", ConsoleOutput::ERROR);
        result->setResult(CRYPT_RESULT::NO_PASSWORD_SET);
        return;
    }

    //create new context for each enc/dec or it will do shits
    //the context is created according to encrypt / decrypt
    EVP_CIPHER_CTX* context = create_context(e);
    if (!context)
    {//error
        result->setResult(CRYPT_RESULT::INIT_FAILED);
        return;
    }

    string word = "Encrypting";
    if (e == Crypto::ENRYPT_DECRYPT::DE)
        word = "Decrypting";

    //if we can use threads (in settigns)
    if (!settings_private->no_threads && ent.getSize() > FILE_SIZE_THREADS)
    {//more than threshold, use threads     
        ConsoleOutput::print(word + " '" + ent.getAbsPath() + "' (in thread)", ConsoleOutput::OK);

        result->runningInThread = true;

        ThreadTask* TT = new ThreadTask(context);

        thread t(&Crypto::aes_cryptFILE, this, ref(ent), context, e, TT->result);

        TT->setThread(move(t));

        //context is now managed and freed in the ThreadTask
        context = nullptr;

        add_thread(TT);
    }
    else
    {
        ConsoleOutput::print(word + " '" + ent.getAbsPath() + "'", ConsoleOutput::OK);

        aes_cryptFILE(ent, context, e, result);
        //delete context
        EVP_CIPHER_CTX_free(context);
    }
}

bool Crypto::crypt_string(const unsigned char* input, size_t in_l, ENRYPT_DECRYPT enc, unsigned char*& out, int& out_len, CryptResult* result) const
{
    //create new context
    EVP_CIPHER_CTX* ctx = create_context(enc);
    
    delete [] out;
    out = nullptr;
    out_len = 0;
    
    if (!ctx)
    {
        result->setResult(CRYPT_RESULT::ENCRYPT_DECRYPT_FAILED);
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    //convert string to char array
    unsigned char* inbuf = (unsigned char*) input;
    
    out = new unsigned char[in_l + BLOCKSIZE + 1];

    //variable to keep track of final size of output
    size_t HowMuchProcessed = 0;
    while (true)
    {
        //result for crypt
        bool res;
        if (enc == ENRYPT_DECRYPT::ENC)
            res = EVP_EncryptUpdate(ctx, out + HowMuchProcessed, &out_len, inbuf, in_l - HowMuchProcessed);
        else
            res = EVP_DecryptUpdate(ctx, out + HowMuchProcessed, &out_len, inbuf, in_l - HowMuchProcessed);

        if (!res)
        {
            result->setResult(CRYPT_RESULT::ENCRYPT_DECRYPT_FAILED);
            EVP_CIPHER_CTX_free(ctx);            
            out_len = 0;
            
            delete [] out;
            out = nullptr;
            return false;
        }
        HowMuchProcessed += out_len;
            break;
        
        //no more data was processed
        if ((size_t) out_len == HowMuchProcessed || out_len == 0)
            break;

        HowMuchProcessed += out_len;
    }

    if (enc == ENRYPT_DECRYPT::ENC)
        EVP_EncryptFinal_ex(ctx, out + HowMuchProcessed, &out_len);
    else
        EVP_DecryptFinal_ex(ctx, out + HowMuchProcessed, &out_len);

    HowMuchProcessed += out_len;
    
    //set length
    out_len = HowMuchProcessed;
        
    EVP_CIPHER_CTX_free(ctx);
    return true;
}

void Crypto::encrypt(const FileDirEntity& e, CryptResult* res)
{
    crypt_file(e, ENRYPT_DECRYPT::ENC, res);
}

void Crypto::decrypt(const FileDirEntity& e, CryptResult* res)
{
    crypt_file(e, ENRYPT_DECRYPT::DE, res);
}

bool Crypto::file_open_read(const string& fname, ifstream& is) const
{
    is.open(fname, ios::in | ios::binary | ios::ate);

    bool res = is.is_open();
    if (!res)
        ConsoleOutput::print("Can not open file. File: " + fname + "does not exist.", ConsoleOutput::ERROR);

    return res;
}

bool Crypto::file_open_write(const string& fname, ofstream& os) const
{
    os.open(fname, ios::out | ios::binary | ios::trunc);

    bool res = os.is_open();
    if (!res)
        ConsoleOutput::print("Can not open file. File: " + fname + "does not exist.", ConsoleOutput::ERROR);

    return res;
}

inline string Crypto::getResultingFileName(ENRYPT_DECRYPT encrypt, const string& pathToFile) const
{
    if (encrypt == ENRYPT_DECRYPT::ENC)
    {
        if (settings_private->ChangeFileNames)
        { //we hash the filename
            string ParentDir = Functions::GetFileParentDir(pathToFile);

            //get only filename with extension from the absolute path
            string Filename = Functions::GetFileNameWithExtension(pathToFile);

            //if we encrypt, hash the filename and set the hash as new filename
            unsigned char* hash = nullptr;
            uint16_t hash_length = 0;
            Hash256(&Filename, hash, hash_length);

            string result = Hash256_ToHumanReadable(hash);

            MemoryWiper::ClearVariable(hash, hash_length);
            MemoryWiper::ClearVariable(&Filename);
            delete [] hash;

            //set the hash as new filename
            return Functions::combinePaths(ParentDir, result) + "." + ENCRYPTED_EXTENSION_;
        }
        else//just add extension (the path is absolute)
            return pathToFile + "." + ENCRYPTED_EXTENSION_;
    }

    //else
    return Functions::removeExtension(pathToFile);
}

bool Crypto::ok() const
{
    return init_ok_private;
}

bool Crypto::SetEncryptionKey(string key)
{
    unsigned char* hash = nullptr;
    uint16_t HashLength = 0;
    Hash256(&key, hash, HashLength);

    //erase it from memory
    MemoryWiper::ClearVariable(&key);
    MemoryWiper::ClearVariable(&Password_hash);

    Password_hash = Hash256_ToHumanReadable(hash);

    MemoryWiper::ClearVariable(hash, HashLength);
    delete [] hash;

    PasswordSet = true;
    return true;
}

void Crypto::Hash256(const string* what, unsigned char*& result, uint16_t& length) const
{
    delete result;
    length = SHA256_DIGEST_LENGTH;

    result = new unsigned char[SHA256_DIGEST_LENGTH];

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, what->c_str(), what->length());
    SHA256_Final(result, &sha256);

    //there is NO null terminator, but we know the length
}

string Crypto::Hash256_ToHumanReadable(unsigned char* hash) const
{
    /* human readable   */
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << hex << setw(2) << setfill('0') << (int) hash[i];

    return ss.str();
}

bool Crypto::readHashFromFile(FILE* fd, bool& correct) const
{//hash the password
    correct = false;
    if (!PasswordSet)
    {
        ConsoleOutput::print("hash to file: Password is not set.", ConsoleOutput::CODE_ERROR);
        return false;
    }

    unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];

    //hash has fixed size
    if (fread(hash, 1, SHA256_DIGEST_LENGTH, fd) != SHA256_DIGEST_LENGTH)
    {
        ConsoleOutput::print("read failed.", ConsoleOutput::FUNCTION_ERROR);
        delete [] hash;
        return false;
    }

    //compare with hash of set password
    correct = (strncmp((const char*) Password_hash.c_str(), (const char*) hash, SHA256_DIGEST_LENGTH) == 0);
        
    MemoryWiper::ClearVariable(hash, SHA256_DIGEST_LENGTH);
    delete [] hash;

    return true;
}

bool Crypto::readFileHead(FILE* in, CryptResult* result, bool& filename_changed, string& originalFileName) const
{
    if (!readHashFromFile_CheckCorrect_PrintIfNot(in, result))
        return false;

    //read whether file name was encrypted
    char enc;
    if (fread(&enc, 1, 1, in) != 1)
    {
        ConsoleOutput::print("read failed. File is corrupted", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }

    filename_changed = (enc == '1');

    if (filename_changed)
    { //read the encrypted file name from file    
        uint32_t length = 0;

        //read length of the encrypted filename
        if (fread((void*) &length, sizeof (uint32_t), 1, in) != 1)
        {
            ConsoleOutput::print("read failed. File is corrupted", ConsoleOutput::FUNCTION_ERROR);
            return false;
        }

        //cout << "head: reading " << length << "( " << sizeof (uint32_t) << "bytes)" << endl;

        //read the file name        
        unsigned char* FileName_buffer = new unsigned char[length];
        if (fread(FileName_buffer, 1, length, in) != length)
        {
            ConsoleOutput::print("read failed. File '" + originalFileName + "' is corrupted", ConsoleOutput::FUNCTION_ERROR);
            return false;
        }
        //decrypt the filename
        unsigned char* decrypted_file_name = nullptr; 
        int decrypted_file_name_l;
        if(!crypt_string(FileName_buffer, length, ENRYPT_DECRYPT::DE, decrypted_file_name, decrypted_file_name_l, result))
        {
            ConsoleOutput::print("Decrypting file name failed. File '" + originalFileName + "' is corrupted", ConsoleOutput::FUNCTION_ERROR);
            return false;
        }
        
        MemoryWiper::ClearVariable(&originalFileName);

        originalFileName = string((const char*)decrypted_file_name, decrypted_file_name_l);
        MemoryWiper::ClearVariable(decrypted_file_name, decrypted_file_name_l);
        delete [] decrypted_file_name;
    }
    return true;
}

inline bool Crypto::readHashFromFile_CheckCorrect_PrintIfNot(FILE* in, CryptResult* result) const
{
    //read hash and check if password is correct
    bool correct;
    if (!readHashFromFile(in, correct))
    {
        result->setResult(CRYPT_RESULT::FILE_READ_FAILED);
        return false;
    }
    if (!correct)
    {
        ConsoleOutput::print("Decryption failed. Password not correct. Stop.", ConsoleOutput::ERROR);
        result->setResult(CRYPT_RESULT::INVALID_PASSWORD);
        return false;
    }
    return true;
}

void Crypto::WaitForFinish(size_t* SucessfullyProcessed)
{
    if (!threads)
        return;

    for (auto i = threads->begin(); i != threads->end(); i++)
    {
        auto t = *i;
        ConsoleOutput::print("Waiting for thread to finish...", ConsoleOutput::WARNING, " * ");
        t->join();

        if (SucessfullyProcessed)
        {
            if (t->succeeded())
                *SucessfullyProcessed += 1;
        }
        delete t;
    }

    delete threads;
    threads = nullptr;
}

inline bool Crypto::writeHashToFile(FILE* fd) const
{
    if (!PasswordSet)
    {
        ConsoleOutput::print("hash to file: Password is not set.", ConsoleOutput::CODE_ERROR);
        return false;
    }

    //write it to the file, on the beginning
    if (fseek(fd, 0, SEEK_SET) != 0)
    {
        ConsoleOutput::print("fseek failed.", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }

    if (fwrite(Password_hash.c_str(), 1, SHA256_DIGEST_LENGTH, fd) != SHA256_DIGEST_LENGTH)
    {
        ConsoleOutput::print("write failed.", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }
    return true;
}

inline bool Crypto::writeFileHead(FILE* fout, CryptResult* result, const string& OriginalFileName) const
{
    //write password hash to file
    if (!writeHashToFile(fout))
    {//some io error
        result->setResult(CRYPT_RESULT::FILE_WRITE_FAILED);
        return false;
    }

    //write whether the filename is going to be encrypted
    char bit = (settings_private->ChangeFileNames) ? '1' : '0';
    if (fwrite(&bit, 1, 1, fout) != 1)
    {
        result->setResult(CRYPT_RESULT::FILE_WRITE_FAILED);
        ConsoleOutput::print("write failed.", ConsoleOutput::FUNCTION_ERROR);
        return false;
    }

    if (settings_private->ChangeFileNames)
    { //encrypt the filename and write it to file        
        string filename = Functions::GetFileNameWithExtension(OriginalFileName);
        
        unsigned char* enced_filename = nullptr;
        int enced_filename_l;
        if(!crypt_string((const unsigned char*)filename.c_str(), filename.length(), ENRYPT_DECRYPT::ENC, enced_filename, enced_filename_l, result))
        {
            ConsoleOutput::print("Encrypting file name failed. File '" + OriginalFileName + "' is corrupted", ConsoleOutput::FUNCTION_ERROR);
            return false;
        }
        MemoryWiper::ClearVariable(&filename);

        //write length of encrypted filename
        uint32_t length = enced_filename_l;
        //write length of the encrypted filename
        if (fwrite((void*) &length, sizeof (uint32_t), 1, fout) != 1)
        {
            result->setResult(CRYPT_RESULT::FILE_WRITE_FAILED);
            ConsoleOutput::print("write failed.", ConsoleOutput::FUNCTION_ERROR);
            
            delete [] enced_filename;
            return false;
        }

        //cout << "head: writing " << length << "( " << sizeof (uint32_t) << "bytes)" << endl;

        //write the encrypted filename bytes
        if (fwrite(enced_filename, 1, length, fout) != length)
        {
            result->setResult(CRYPT_RESULT::FILE_WRITE_FAILED);
            ConsoleOutput::print("write failed.", ConsoleOutput::FUNCTION_ERROR);
            
            delete [] enced_filename;
            return false;
        }
        delete [] enced_filename;
    }
    //done
    return true;
} 