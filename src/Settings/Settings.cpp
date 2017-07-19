#include "Settings.h"

Settings::Settings() : 
        ChangeFileNames(false),
        ChangeFileNames_option_found(false),
        CheckForUnEncryptedFiles(false),
        encrypt(false),
        EncryptAllExtensions(false),
        encrypt_or_decrypt_option_found(false),
        exitProgram_private(false),
        exitcode_private(0),
        find_encrypted(false),
        no_delete(false),
        no_threads(false),
        printAllOptions(false),
        printHelp(false),
        printVersion(false),
        recursive(false),
        runTests(false),
        verbose(false)
{}
Settings::~Settings()
{}

bool Settings::AddFileName(string&& f)
{
    return FilesToEcrypt_private.insert(f).second;
}
bool Settings::decrypt() const
{
    return !encrypt;
}  
int Settings::ExitCode() const
{
    return exitcode_private;
}
bool Settings::ExitProgram() const
{
    return exitProgram_private;
}  
 const unordered_set<string>* Settings::GetFilesOrDirsToEncryptDecrypt() const
 {
     return &FilesToEcrypt_private;
 }
void Settings::SetExitProgram_And_ExitCode(int exitcode)
{
    exitProgram_private = true;
    exitcode_private = exitcode;    
}
