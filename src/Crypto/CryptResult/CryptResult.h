#pragma once

#include "../Crypto.h"

/**
 * @brief describes the result of en / de crypting process
 * @param RunsInThread
 */
struct CryptResult
{
    CryptResult();
    /**
     * @brief Creates result for operation that has finished, and has result
     * @param r The result to set
     */
    CryptResult(Crypto::CRYPT_RESULT r);
    /**
     * @brief Creates result for operation that is running in thread (so there is no result yet). 
     * @param b Must be true
     */
    CryptResult(bool b);
    
    ~CryptResult();
    /**
     * @brief Creates result for operation that is running in thread (so there is no result yet)
     */
    static CryptResult createRunningInThread();
    //================================================
    
    /**
     * 
     * @return Undefined if still running in thread and not finished 
     */
    Crypto::CRYPT_RESULT getResult() const;
    void setResult(Crypto::CRYPT_RESULT r);
    //================================================
private:
    Crypto::CRYPT_RESULT result_private;
public:
    string final_filename;
    bool runningInThread;
};