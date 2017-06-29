#include "CryptResult.h"

#include <assert.h>

CryptResult::CryptResult() : runningInThread(false)
{}
CryptResult::CryptResult(Crypto::CRYPT_RESULT r) : result_private(r), runningInThread(false)
{
}

CryptResult::CryptResult(bool b) : runningInThread(true)
{
    if (!b)
        assert(false);
}

CryptResult::~CryptResult()
{
}

CryptResult CryptResult::createRunningInThread()
{
    return CryptResult(true);
}

Crypto::CRYPT_RESULT CryptResult::getResult() const
{
    return result_private; 
}    

void CryptResult::setResult(Crypto::CRYPT_RESULT r)
{
    runningInThread = false;
    result_private = r;
}