#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <unordered_set>

using namespace std;

/**
 * @brief Specifies settings struture for holding setup according to arguments
 */
struct Settings {
    Settings();
    Settings(const Settings& a) = delete;
    ~Settings();

    bool AddFileName(string&& f);
    bool decrypt() const;

    const unordered_set<string>* GetFilesOrDirsToEncryptDecrypt() const;

    Settings& operator=(const Settings& s) = delete;

    void SetExitProgram_And_ExitCode(int exitcode);
    int ExitCode() const;
    bool ExitProgram() const;

    bool CheckForUnEncryptedFiles; /**< Whether application should look for any file that is not ecrypted and print it. */
    bool encrypt; /**< Whether files should be encrypted or decrypted */
    bool EncryptAllExtensions; /**< Whether even files ending with encrypted extension shall be encrypted*/
    bool encrypt_or_decrypt_option_found;
    
private:
    bool exitProgram_private; /**< Program should exit, because help or version was print */
    int exitcode_private;

    unordered_set<string> FilesToEcrypt_private;

public:
    bool find_encrypted;
    bool no_delete; /**< Whether encrypted / decrypted file (original) should not be deleted*/
    bool printHelp;
    bool printVersion;
    bool recursive;
    bool runTests; /**< Whether tests should be ran*/
    bool verbose;
};

#endif /* SETTINGS_H */

