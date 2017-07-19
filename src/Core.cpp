#include "Core.h"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <termios.h>

#include <assert.h>
#include <unordered_map>

#include "ArgumentPasser.h"
#include "Crypto/Crypto.h"
#include "Tester.h"
#include "MemoryWiper.h"
#include "ConsoleOutput/ConsoleOutput.h"
#include "Colors/Colors.h"
#include "Functions.h"
#include "FileSystem/FileDirEntity.h"
#include "Crypto/CryptResult/CryptResult.h"
#include "shred/Shred.h"
#include "cmdOptions/cmdOptions.h"


char Core::VERSION[] = "2.2";
/**
 * Notes
 * crypt_string will have undefined behaviour if length of string exceeds max of length of int
 * 
 */
Core::Core() : argc_allocated(0), argv_allocated(nullptr), SimulatedArguments(false)
{
}

Core::~Core()
{
    if (SimulatedArguments)
        FreeAllocatedArguments();
}

inline void Core::FreeAllocatedArguments()
{
    for (int i = 0; i < argc_allocated; i++)
    {
        delete [] argv_allocated[i];
    }
    delete [] argv_allocated;
    argv_allocated = nullptr;
    argc_allocated = 0;
}

inline bool Core::GetEncryptionKey(string& pass) const
{
    ConsoleOutput::print("Enter password:", ConsoleOutput::QUESTION);
    HideStdinKeystrokes();

    cin >> pass;

    /* if (pass.length() < 5)
     {
         MemoryWiper::ClearVariable(&pass);
         cerr << "Invalid password. Must be minimum length of 5 characters" << endl;


         ShowStdinKeystrokes();
         return false;
     }*/
    string pass2;

    ConsoleOutput::print("Password again:", ConsoleOutput::QUESTION);
    cin >> pass2;

    if (pass2 != pass)
    {
        ConsoleOutput::print("Passwords do not match", ConsoleOutput::ERROR);

        MemoryWiper::ClearVariable(&pass);
        MemoryWiper::ClearVariable(&pass2);


        ShowStdinKeystrokes();
        return false;
    }
    MemoryWiper::ClearVariable(&pass2);

    ShowStdinKeystrokes();
    return true;
}

inline string Core::getfullexepath(char* argv_path) const
{
    //get length
    string s(argv_path);

    char the_path[PATH_MAX];
    getcwd(the_path, PATH_MAX);

    s += "/" + string(the_path);

    return s;
}

inline void Core::HideStdinKeystrokes() const
{
    termios tty;

    tcgetattr(STDIN_FILENO, &tty);

    /* we want to disable echo */
    tty.c_lflag &= ~ECHO;

    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

inline void Core::printOptionHelp(string&& option, string&& decription) const
{
    cout << "\t" << Colors::BOLD << option << Colors::RESET << "\t" << decription << endl << endl;
}

inline int Core::printHelp(const cmdOptions* options) const
{
    cout << "\t\t" << Colors::BOLD << "Man for " << APP_NAME << " (ver. " << VERSION << ")" << Colors::RESET << endl;

    cout << endl;
    cout << "This application was written by " << Colors::BOLD << "Jan Glaser" << Colors::RESET << ". Author is not responsible for any damage you cause to anyone using this program." << endl;
    cout << endl;
    cout << Colors::BOLD << "NAME" << Colors::RESET << endl;
    cout << "\t" << APP_NAME << " - Allows to encrypt or decrypt file or directories using AES 256 CBC." << endl << endl;
    cout << Colors::BOLD << "SYNOPSIS" << Colors::RESET << endl;
    cout << "\t" << APP_NAME << " [" << Colors::UNDERLINE << "OPTIONS" << Colors::RESET << "] [" << Colors::UNDERLINE << "FILES / DIRS" << Colors::RESET << "]" << endl << endl;
    cout << Colors::BOLD << "DESCRIPTION" << Colors::RESET << endl;
    cout << "\t" << APP_NAME << "  is  used  for  encrypting or decrypting file or directory content. AES256 " <<
            "in CBC mode is used for the task. Keys and sensitive data is being overwritten as soon as possible in " <<
            "memory." << endl << endl;
    cout << "\t" << "Threads are used for en / de cryption, if the file exceeds size of " << Colors::UNDERLINE << Crypto::FILE_SIZE_THREADS <<
            Colors::RESET << " Bytes." << endl;


    cout << Colors::BOLD << "OPTIONS" << Colors::RESET << endl;
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::ENCRYPT), "Encrypts target file(s) or directories (non recursively)");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::DECRYPT), "Decrypts target file(s) or directories (non recursively)");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::RECURSIVE), "Recursive. Encrypts / decrypts / performs check on content of directory recursively");

    cout << Colors::BOLD << "[FILES/DIRS]" << Colors::RESET << endl;
    cout << "\tSpace  separated  files  (absolute  or relative path) that will be encrypted or " <<
            "decrypted. If a directory is passed, all  content  of  such  a  directory  is " <<
            "encrypted  /  decrypted " << Colors::BOLD << "non recursively by default" << Colors::RESET << ", including all links and symlinks and " << Colors::UNDERLINE <<
            "including" << Colors::RESET << " hidden files." << endl;
    cout << "\t" << "Unless --force option is present, no file ending with ." << Crypto::ENCRYPTED_EXTENSION_ << " will be encrypted to avoid double encryption of same file." << endl;
    cout << "\t(see " << Colors::UNDERLINE << "ENCRYPTED EXTENSION" << Colors::RESET << ")" << endl << endl;

    cout << endl;

    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::CHANGE_NAMES_Y), "When encrypting file, the file name is also encrypted to the final file. Upon decryption, the original fine name is restored (see " + Colors::UNDERLINE + "EXAMPLES" + Colors::RESET + " for more detailed explanation.");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::CHANGE_NAMES_N), "This is used by default. When encrypting file, the file name is preserved, only '" + Crypto::ENCRYPTED_EXTENSION_ + "' extension is added.");

    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::CHECK), "Checks whether file(s) or directory content is encrypted. Check is done only against the extension of files, not against content.");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::FORCE), "Encrypts also files that are ending with encrypted extension (" +
            Crypto::ENCRYPTED_EXTENSION_ + "), and decrypts (or tries to) also files that do not end with such an extension");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::FIND), "Prints all files that do have ." + Crypto::ENCRYPTED_EXTENSION_ + " extension.");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::HELP), "Shows this help and quits");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::NO_DELETE), "After encryption or decryption, the original input file is not deleted");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::NO_THREADS), "Threads will not be used for encryption or decryption of files");
   
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::PRINT_ALL_OPTIONS), "Prints all available options");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::TESTS), "Performs hard coded tests");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::VERSION), "Prints version and quits");
    printOptionHelp(options->getOptionText(cmdOptions::OPTION_NAME::VERBOSE), "Verbose. Prints additional output");

    cout << endl;
    cout << Colors::BOLD << "ENCRYPTION" << Colors::RESET << endl;
    cout << "\t" << "For all arguments that are passed on command line as " << Colors::BOLD << "[FILES/DIRS]" << Colors::RESET << endl;
    cout << "\t" << "All directories are traversed, adding all files found to list (including hidden files, symlinks)" << endl;
    cout << "\t" << "To the list, other files from command lines are also added" << endl;
    cout << "\t" << "If -r is specified, this action (directories) is done recursively" << endl;
    cout << "\t" << "If file from final list is about to be encrypted and " << Colors::BOLD << "does " <<
            Colors::RESET << "contain [." << Crypto::ENCRYPTED_EXTENSION_ << "] extension, it is omitted, unless --force option is present." << endl;
    cout << "\t" << "If file is about to be decrypted and does" << Colors::BOLD << " not" << Colors::RESET <<
            " contain extension, it is omitted, unless --force option is present." << endl;

    cout << endl;
    cout << Colors::BOLD << "SECURITY AND ALGORITHM" << Colors::RESET << endl;
    cout << "\t" << APP_NAME << " uses AES in CBC mode, using 256 bit key." << endl << endl;
    cout << "\t" << "Input pass phrase is hashed using SHA256, end KDV is used to get key and IV. Salt is also used, salt is derived from hashed pass phrase." << endl;
    cout << "\tThe target file is encrypted using the mentioned block cipher. Hash of the pass phrase is added to the encrypted file. " <<
            "That way, when decrypting file, incorrect password can be determined." << endl;


    cout << endl;
    cout << Colors::BOLD << "ENCRYPTED EXTENSION" << Colors::RESET << endl;
    cout << "\t" << APP_NAME << " uses fixed extension for all encrypted files. ( *." << Crypto::ENCRYPTED_EXTENSION_ << " )" << endl << endl;
    cout << "\t" << "When file is encrypted, the extension is added to the name." << endl;
    cout << "\t" << "When decrypting, the extension is removed. ";
    cout << "(see " << Colors::UNDERLINE << "ENCRYPTION" << Colors::RESET << " for additional in-depth behaviour)" << endl;


    cout << endl;
    cout << Colors::BOLD << "EXAMPLES" << Colors::RESET << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -c file1.cpp file2.cpp" << Colors::RESET << endl;
    cout << "\t\t" << "Encrypts both files file1.cpp file2.cpp, if they exist" << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -d file1.cpp file2.cpp" << Colors::RESET << endl;
    cout << "\t\t" << "Decrypts both files file1.cpp file2.cpp, if they exist" << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -d file1.cpp directory" << Colors::RESET << endl;
    cout << "\t\t" << "Encrypts file file1.cpp and content of directory 'directory' non recursively" << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -c . ../" << Colors::RESET << endl;
    cout << "\t\t" << "Encrypts  all files non recursively in current directory and parent directory, but" <<
            " encrypting each file only once, not multiple times" << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -cr . ../" << Colors::RESET << endl;
    cout << "\t\t" << "Encrypts  all files recursively in current directory and parent directory, but" <<
            " encrypting each file only once, not multiple times" << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -c file1.PH file2.cpp" << Colors::RESET << endl;
    cout << "\t\t" << "Encrypts file file2.cpp, if it exists (see " << Colors::UNDERLINE << "ENCRYPTED EXTENSION)" << Colors::RESET << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -c --force file1.PH file2.cpp" << Colors::RESET << endl;
    cout << "\t\t" << "Encrypts files file1.PH and file2.cpp, if they exist (see " << Colors::UNDERLINE << "ENCRYPTED EXTENSION)" << Colors::RESET << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -d file1.PH file2.cpp" << Colors::RESET << endl;
    cout << "\t\t" << "Decrypts file file1.PH, if it exists (see " << Colors::UNDERLINE << "ENCRYPTED EXTENSION)" << Colors::RESET << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -d --force file1.PH file2.cpp" << Colors::RESET << endl;
    cout << "\t\t" << "Decrypts files (or tries to) file1.PH and file2.cpp, if they exist (see " << Colors::UNDERLINE << "ENCRYPTED EXTENSION)" << Colors::RESET << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -c --change-names=y -r ." << Colors::RESET << endl;
    cout << "\t\t" << "Encrypts all files recursively in current directory (except symbolic links and encrypted files)." << endl <<
            "Also changes the original file name of each file to a hash. The original filename is encrypted and stored to file. Upon decryption, all filenames will be restored." << endl;
    cout << "\t" << Colors::BOLD << APP_NAME << " -dr --change-names=n ." << Colors::RESET << endl;
    cout << "\t\t" << "Decrypts all files recursively in current directory (except symbolic links and un encrypted files)." << endl <<
            "The option --change-names=n has no effect. If the encrypted file contains encrypted original file name, it will always be restored." << endl;

    cout << endl;
    cout << Colors::BOLD << "BUGS" << Colors::RESET << endl;
    cout << "\t" << "There are no known bugs, yet. If you find one, report it to github (" << Colors::UNDERLINE << "https://github.com/Horse303/cryptor" << Colors::RESET << ")" << endl;

    cout << endl;
    return 0;
}

inline int Core::printVersion() const
{
    cout << "Version " << VERSION << endl;
    return 0;
}

int Core::run(int argc, char** argv)
{
    cmdOptions cmdLineOptions;
    
    //SimulateArgs(argc, argv, {"--print-all-options"});
    //SimulateArgs(argc, argv, {"-cr", "--change-names=y", "/root/Desktop/9905bac1fbd114cb94aaf4765b282af6.png"});
    //SimulateArgs(argc, argv,{"-d", "/root/Desktop/abc.PH"});
    Settings s;
    ArgumentPasser AP;
    if (!AP.ParseArguments(argv, argc, &s, &cmdLineOptions))
        return 1;

    if (s.ExitProgram())
        return s.ExitCode();
    else if (s.runTests)
    {
        string abs_p;
        Functions::getFileDirAbsolutePath(argv[0], abs_p);
        return RunTests(Functions::GetFileParentDir(abs_p), s);
    }
    else if(s.printAllOptions)
    {
        cout << cmdLineOptions.getAllOptionTextString() << endl;
        return 0;
    }
    else if (s.printVersion)
        return printVersion();
    else if (s.printHelp)
        return printHelp(&cmdLineOptions);

    if (s.GetFilesOrDirsToEncryptDecrypt()->size() == 0)
    {
        ConsoleOutput::print("No files or dirs specified. Quit.", ConsoleOutput::ERROR);
        return 1;
    }

    Crypto C(s);
    /*C.SetEncryptionKey("key");
    string enc = C.crypt_string("some text", Crypto::ENRYPT_DECRYPT::ENC, new CryptResult());
    cout << C.crypt_string(enc, Crypto::ENRYPT_DECRYPT::DE, new CryptResult()) << endl;
    
    assert(false);
     */
    if (!s.CheckForUnEncryptedFiles && !s.find_encrypted)
    {
        if (!s.encrypt_or_decrypt_option_found)
        {
            ConsoleOutput::print("No -c or -d option specified. Quit.", ConsoleOutput::ERROR);
            return 1;
        }

        //if we want to just see files that are not encrypted, no key and setup of encryption is needed
        string key = "";
        if (!GetEncryptionKey(key))
            return 1; //here var is cleaned already

        C.SetEncryptionKey(key);
        MemoryWiper::ClearVariable(&key);
    }

    //get the files and dirs from command line, and all files according to settings and filters (extensions, recursivity)
    FileSystem FS;

    if (s.verbose)
        ConsoleOutput::print("Preparing list of files to work with ...", ConsoleOutput::OK);


    auto Files = FS.FindAllFiles(s);

    //If checking only for un encrypted or encrypted files, all was printed in the FS.FindAllFiles
    if (s.CheckForUnEncryptedFiles || s.find_encrypted)
        return 0;


    size_t total_files = Files.size();

    if (s.encrypt)
        cout << "Preparing to encrypt " << total_files << " files..." << endl;
    else
        cout << "Preparing to decrypt " << total_files << " files..." << endl;

    if (Files.size() >= 500)
    {
        ConsoleOutput::print("You are about to encrypt more than 500 files. Are you sure? (y / n)", ConsoleOutput::WARNING);
        string input;

        while (true)
        {
            if (input == "n")
            {
                ConsoleOutput::print("Aborted", ConsoleOutput::OK);
                return 0;
            }
            else if (input == "y")
                break;

            cout << "Type 'y' or 'n'" << endl;
            cin >> input;
        }

    }
    bool res = true;
    size_t SucessfullyProcessed = 0;
    for (auto i = Files.begin(); i != Files.end(); i++)
    {
        auto ent = i->second;
        string path = ent->getAbsPath();

        CryptResult result_info;
        if (s.encrypt)
        {
            if (!s.EncryptAllExtensions && Functions::getExtension(path) == C.ENCRYPTED_EXTENSION_)
            {
                ConsoleOutput::print("Skipping apparently encrypted file '" + path + "'", ConsoleOutput::WARNING);
                continue;
            }
            //ConsoleOutput::print("Encrypting '" + path + "'", ConsoleOutput::OK);
            C.encrypt(*ent, &result_info);
        }
        else
        {
            if (!s.EncryptAllExtensions && Functions::getExtension(path) != C.ENCRYPTED_EXTENSION_)
            {
                ConsoleOutput::print("Skipping apparently un encrypted file '" + path + "'", ConsoleOutput::WARNING);
                continue;
            }

            //ConsoleOutput::print("Decrypting '" + path + "'", ConsoleOutput::OK);
            C.decrypt(*ent, &result_info);
        }

        if (!result_info.runningInThread)
        {//we have result already
            if (result_info.getResult() == Crypto::CRYPT_RESULT::OK)
                SucessfullyProcessed++;
            else
                break;
        }
    }

    //MAKE SURE, VARIABLE [Files] EXISTS STILL UNTIL ALL THREADS FINISH, SINCE THEZ CONTAIN REFERENCES TO CONTENT
    //OF THE ARRAY
    //wait for threads to finish
    C.WaitForFinish(&SucessfullyProcessed);

    if (s.encrypt)
        ConsoleOutput::print("Successfully encrypted [ " + to_string(SucessfullyProcessed) + " / " + to_string(total_files) + "] files", ConsoleOutput::OK, " OK ");
    else
        ConsoleOutput::print("Successfully decrypted [ " + to_string(SucessfullyProcessed) + " / " + to_string(total_files) + "] files", ConsoleOutput::OK, " OK ");

    return res;
}

inline int Core::RunTests(string&& runningdirectory, const Settings& s) const
{
    Tester t(move(runningdirectory), s);
    t.verbose = true;
    if (t.run(1, 10000))
        return 0;

    return 1;
}

inline void Core::ShowStdinKeystrokes() const
{
    termios tty;

    tcgetattr(STDIN_FILENO, &tty);

    /* we want to re enable echo */
    tty.c_lflag |= ECHO;

    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

inline void Core::SimulateArgs(int& argc, char**& argv, initializer_list<string> args)
{
    if (args.size() < 1)
        return;

    if (SimulatedArguments)
        FreeAllocatedArguments();

    int new_argc = 1 + args.size();

    char** newarr = new char*[new_argc];
    size_t index = 1;
    newarr[0] = new char[strlen(argv[0]) + 1];
    strcpy(newarr[0], argv[0]);
    for (auto i = args.begin(); i != args.end(); i++)
    {
        newarr[index] = new char[(*i).length() + 1];
        strcpy(newarr[index], (*i).c_str());
        index++;
    }

    argc = new_argc;
    argv = newarr;
    argv_allocated = argv;
    argc_allocated = argc;

    SimulatedArguments = true;
} 
