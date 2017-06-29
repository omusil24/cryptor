#include "ThreadTask.h"

#include "CryptResult/CryptResult.h"

#include <assert.h>

ThreadTask::ThreadTask(EVP_CIPHER_CTX* contex) : context(contex), result(new CryptResult(true)), thread_set(false)
{
    
}

ThreadTask::~ThreadTask()
{
    delete result;
    EVP_CIPHER_CTX_free(context);
}

void ThreadTask::join()
{
    if(!thread_set)
        return;
    working_thread.join();
    
    EVP_CIPHER_CTX_free(context);
    context = nullptr;
}
void ThreadTask::setThread(thread&& t)
{
    if(thread_set)
        assert(false);
    thread_set = true;
    working_thread = move(t);
}
bool ThreadTask::succeeded() const
{
    return (result->getResult() == Crypto::CRYPT_RESULT::OK);
}