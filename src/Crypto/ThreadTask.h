#pragma once

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <thread>

#include "Crypto.h"

class CryptResult;

using namespace std;

/**
 * @brief Used to hold data about task for crypto that is being done in thread
 */
struct ThreadTask
{
    ThreadTask(EVP_CIPHER_CTX* context);
    ThreadTask(const ThreadTask& a) = delete;
    ~ThreadTask();
    
    void join();
    void setThread(thread&& t);
    bool succeeded() const;
    
private:
    EVP_CIPHER_CTX* context;
public:
    CryptResult* result;
private:
    bool thread_set;
    thread working_thread;
};