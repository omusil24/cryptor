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

const string Crypto::ENCRYPTED_EXTENSION_ = "PH";
const size_t Crypto::FILE_SIZE_THREADS = 500000; 

Crypto::Crypto(const Settings& s) : 
init_ok_private(true), PasswordSet(false), settings_private(&s), threads(nullptr)
{
}

Crypto::~Crypto()
{
    WaitForFinish();

    /* Clean up */
    MemoryWiper::ClearVariable(&Password_hash);

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


void Crypto::aes_cryptFILE(string pathToFile, EVP_CIPHER_CTX* ctx, Crypto::ENRYPT_DECRYPT encrypt, CryptResult* result)
{   
    string word = "Encrypting";
    if(encrypt == Crypto::ENRYPT_DECRYPT::DE)
        word = "Decrypting";
            
    if(result->runningInThread)        
        ConsoleOutput::print(word + " '" + pathToFile + "' (in thread)", ConsoleOutput::OK);
    else
        ConsoleOutput::print(word + " '" + pathToFile + "'", ConsoleOutput::OK);
    
    EVP_CIPHER_CTX* e = ctx;

    FILE* in;
    in = fopen(pathToFile.c_str(), "rb");
    if (in == nullptr)
    {
        ConsoleOutput::print("Failed to open file.", ConsoleOutput::ERROR);
        result->setResult(CRYPT_RESULT::FILE_OPEN_FAILED);
        return;
    }


    if (encrypt == ENRYPT_DECRYPT::DE)
    {
        //read hash and check if password is correct
        bool correct;
        if (!readHashFromFile(in, correct))
        {
            result->setResult(CRYPT_RESULT::FILE_READ_FAILED);
            return;
        }
        if (!correct)
        {
            ConsoleOutput::print("Decryption failed. Password not correct. Stop.", ConsoleOutput::ERROR);
            result->setResult(CRYPT_RESULT::INVALID_PASSWORD);
            return;
        }

        //if pass is correct, now position in file is behind hash so we can continue reading....        
    }

    unsigned char* inbuf = new unsigned char[READ_CHUNK];
    unsigned char* outbuf = new unsigned char[READ_CHUNK + BLOCKSIZE];
    int out_len = 0;

    FILE* fout = nullptr;

    string ResultingFileName;

    if (encrypt == ENRYPT_DECRYPT::ENC)
        ResultingFileName = pathToFile + "." + ENCRYPTED_EXTENSION_;
    else
        ResultingFileName = Functions::removeExtension(pathToFile);

    fout = fopen(ResultingFileName.c_str(), "wb");
    if (!fout)
    {
        delete [] outbuf;
        closeFile(in);
        ConsoleOutput::print("Failed to create a new file.", ConsoleOutput::ERROR);
        result->setResult(CRYPT_RESULT::FILE_CREATE_FAILED);
        return;
    }
    if (encrypt == ENRYPT_DECRYPT::ENC)
    {//we are encrypting, write password hash to file
        if (!writeHashToFile(fout))
        {//some io error
            delete [] outbuf;
            closeFile(in);
            result->setResult(CRYPT_RESULT::FILE_WRITE_FAILED);
            return;
        }
    }

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
        if(e == ENRYPT_DECRYPT::ENC)
            EVP_EncryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv);
        else
            EVP_DecryptInit_ex(context, EVP_aes_256_cbc(), NULL, key, iv);
    }
    MemoryWiper::ClearVariable(&nrounds);
    MemoryWiper::ClearVariable(&key[0], 32);
    MemoryWiper::ClearVariable(&iv[0], 16);
    MemoryWiper::ClearVariable(&salt[0], 8);

    if(!status)
    {
        EVP_CIPHER_CTX_free(context);
        return nullptr;
    }
    return context;
}
void Crypto::crypt_file(const FileDirEntity& ent, ENRYPT_DECRYPT e, CryptResult* result)
{
    if(!result)
    {
        ConsoleOutput::print("Logic error, result is null", ConsoleOutput::CODE_ERROR);
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
    if(!context)
    {//error
        result->setResult(CRYPT_RESULT::INIT_FAILED);
        return;
    }
        
    if(ent.getSize() > FILE_SIZE_THREADS)
    {//more than threshold, use threads     
                
        result->runningInThread = true;
        
        ThreadTask* TT = new ThreadTask(context);
                    
        thread t(&Crypto::aes_cryptFILE, this, ent.getAbsPath(), context, e, TT->result);
        
        TT->setThread(move(t));
                
        //context is now managed and freed in the ThreadTask
        context = nullptr;
    
        add_thread(TT);
    }
    else
    {
        aes_cryptFILE(ent.getAbsPath(), context, e, result);
        //delete context
        EVP_CIPHER_CTX_free(context);
    }
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
    
    //allocate bigger array
    char* hash2 = new char[HashLength + 1];
    strncpy(hash2, (const char*)hash, HashLength);
    hash2[HashLength] = '\0';
    
    MemoryWiper::ClearVariable(hash, HashLength);
    delete [] hash;
    hash = (unsigned char*)hash2;
    hash2 = nullptr;
    
    Password_hash = string((const char*) hash);

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
            if(t->succeeded())
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