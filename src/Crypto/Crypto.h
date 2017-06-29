#pragma once

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <string>
#include <list>
#include <thread>

#include "../FileSystem/FileDirEntity.h"

class CryptResult;
class Settings;
class ThreadTask;

using namespace std;

/**
 * @brief Class for encrypting and decrypting files using AES cipher, 256 bit bey in CBC mode.
 * When working with file of size greater than [...] then encryption / decryption is done in thread.
 * Make sure you call WaitForFinish to ensure all threads finish.
 * However, that method is called anyway upon destruction of this object
 */
struct Crypto
{
    enum ENRYPT_DECRYPT
    {
        ENC,
        DE
    };
    enum CRYPT_RESULT
    {
        FUNCTION_FAILURE,
        DECRYPT_FAILED_INCORRECT_FILESIZE,
        ENCRYPT_DECRYPT_FAILED,
        FILE_CLOSE_FAILED,
        FILE_CREATE_FAILED,
	FILE_OPEN_FAILED,
	FILE_READ_FAILED,	
	FILE_WRITE_FAILED,
	INIT_FAILED,
        INVALID_PASSWORD,
        NO_PASSWORD_SET,
    	OK
    };
public:
    Crypto(const Settings& s);
    virtual ~Crypto();
        
private:
    inline void add_thread(ThreadTask* t);
    
    void aes_cryptFILE(string pathToFile, EVP_CIPHER_CTX* ctx, ENRYPT_DECRYPT encrypt, CryptResult* result);
        
    bool closeFile(FILE* f) const;
    EVP_CIPHER_CTX* create_context(ENRYPT_DECRYPT e) const;
    
    void crypt_file(const FileDirEntity& en, ENRYPT_DECRYPT e, CryptResult* result);
    
public:
    /**
     * @brief Decrypts target file
     * @param path_to_file
     * @param res Returns the result of decryption, if specified
     */
    void decrypt(const FileDirEntity& e, CryptResult* res);
    
    /**
     * @brief Encrypts target file
     * @param path_to_file
     * @param res Returns the result of encryption, if specified
     */
    void encrypt(const FileDirEntity& e, CryptResult* res);
      
private:
    /**
    * @brief Opens file and sets up ifstream object for reading binary.
    * @param is stream to modify
    * @return Whether file was successfully opened or not.
    */
    bool file_open_read(const string& fname, ifstream& is) const;

    /**
    * @brief Opens file and sets up ifstream object for writing binary. Creates file if it does not exist, or overwrites the existing file.
    * @param is stream to modify
    * @return Whether file was successfully opened or not.
    */
    bool file_open_write(const string& fname, ofstream& os) const;
    
    void Hash256(const string* what, unsigned char*& result, uint16_t& length) const;

public:
    bool ok() const;
    
    bool SetEncryptionKey(string key);

private:
    /**
     * 
     * @param fd Descriptor of file
     * @param correct whether the hashed password matches this set password
     * @return Whether function finished without errors 
     */
    bool readHashFromFile(FILE* fd, bool& correct) const;
public:
    /**
     * @brief waits for all unfinished threads to finish
     * @param SucessfullyProcessed Modifies variable incrementing by 1 for each additional successful operation (in threads). Can be null
     */
    void WaitForFinish(size_t* SucessfullyProcessed = nullptr);
private:    
    inline bool writeHashToFile(FILE* fd) const;
    
    const int BLOCKSIZE = 16;
    const uint16_t READ_CHUNK = 1024; /**< How much read from file at once */
    
public:   
    static const string ENCRYPTED_EXTENSION_;
    
private:
    bool init_ok_private; /**< Whether initialization was ok */

public:
    static const size_t FILE_SIZE_THREADS; /**< When working with 0,5 GB file or greater, threads will be used */
    
private:
    string Password_hash; /**< Password from the user (modified, hashed) */
    bool PasswordSet; /**< Whether password was set in this instance for de / en cryption */
    const Settings* settings_private;
    
    list<ThreadTask*>* threads; /**< List of working threads. can be null (empty)*/
};
