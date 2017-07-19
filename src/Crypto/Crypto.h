#pragma once

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <string.h>
#include <string>
#include <list>
#include <thread>
#include <mutex>

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
   
    /**
     * Encrypts or decrypts content of provided file
     * @param pathToFile
     * @param ctx
     * @param encrypt
     * @param result
     */
    void aes_cryptFILE(const FileDirEntity& en, EVP_CIPHER_CTX* ctx, ENRYPT_DECRYPT encrypt, CryptResult* result);
      
    /**
     * Closes a file. If error is occured, message is printed to output and false is returned.
     * @param f File handle to close
     * @return Whether it succeded without any problems
     */
    bool closeFile(FILE* f) const;
    /**
     * 
     * @param e
     * @return 
     */
    EVP_CIPHER_CTX* create_context(ENRYPT_DECRYPT e) const;
    
    void crypt_file(const FileDirEntity& en, ENRYPT_DECRYPT e, CryptResult* result);
public:
     /**
     * Method used for only encrypting / decrypting a string. Implemented to handle even very long strings.
     * The encrypt / decrypt context is created according to the password hash set it this class.
     * If the set password is NOT valid, result is not defined.
     * @param input String to encrypt / decrypt
     * @param e operation to perform
      * @param out The array to put output in. This will be deleted and then new space allocated. It is up to caller to free it afterwards
      * * @param out_len Length of the output
     * @param result Structure to which will be set status of operation (fail / success)  
     * @return Whether operation succeded or not
     */
    bool crypt_string(const unsigned char* input, size_t in_l, ENRYPT_DECRYPT enc, unsigned char*& out, int& out_len, CryptResult* result) const;
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
private:
    string getResultingFileName(ENRYPT_DECRYPT e, const string& path) const; 
    /**
     * Hashes the provided string using sha 256. The result is NOT null terminated, but you know the length.
     * The null terminator can be there, since the byte can occur to be \0
     * @param what String to hash. This is input
     * @param result Here will be stored final hash. on this pointer is called delete, and new space is allocated. Then value is stored
     * @param length The final length of the result (length of hash)
     */
    void Hash256(const string* what, unsigned char*& result, uint16_t& length) const;

    inline string Hash256_ToHumanReadable(unsigned char* hash) const;
    
    
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
    /**
     * Reads the head of encrypted file. Reads wheher file was symlink and checks for password correctness
     * @param in
     * @param result
     * @param RedFileName Here is returned decrypted original file name (only name with extension, not absolute path)
     * @return Whether all is ok and password was correct
     */
    inline bool readFileHead(FILE* in, CryptResult* result, bool& filename_changed, string& RedFileName) const;
    /**
     * Reads hash from the file and checks whether password is correct. If not, message is printed and false returned.
     * @param in Stream for reading the file
     * @return Whether all is ok and decryption can continue
     */
    inline bool readHashFromFile_CheckCorrect_PrintIfNot(FILE* in, CryptResult* result) const;
public:
    /**
     * @brief waits for all unfinished threads to finish
     * @param SucessfullyProcessed Modifies variable incrementing by 1 for each additional successful operation (in threads). Can be null
     */
    void WaitForFinish(size_t* SucessfullyProcessed = nullptr);
private:    
    inline bool writeHashToFile(FILE* fd) const;
    /**
     * Writes head to the encrypted file.
     * This includes hash, byte saying whether filename was encrypted, and if so, then the encrypted original file name
     * @param f
     * @param ctx
     * @param result
     * @param OriginalFileName Absolute path to the file that is being encryted.
     * @return 
     */
    inline bool writeFileHead(FILE* f, CryptResult* result, const string& OriginalFileName) const;
    
    const int BLOCKSIZE = 16;
    const uint16_t READ_CHUNK = 1024; /**< How much read from file at once */
    
public:   
    static const string ENCRYPTED_EXTENSION_;
    
private:
    bool init_ok_private; /**< Whether initialization was ok */

public:
    static const size_t FILE_SIZE_THREADS; /**< When working with 0,5 GB file or greater, threads will be used */
    
private:
    mutex mtx_file_create; /**< mutex for when check is being done whether the file exists, and then creating nedw file. */
    string Password_hash; /**< Password from the user (modified, hashed) */
    bool PasswordSet; /**< Whether password was set in this instance for de / en cryption */
    const Settings* settings_private;
    
    list<ThreadTask*>* threads; /**< List of working threads. can be null (empty)*/
};
