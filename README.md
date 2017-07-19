#cryptor 2.2

this project was written by Jan Glaser

You can freely copy or redistribute the program, but provide there my name, as author, please

The main idea of the program is to allow safe encryption / decryption of files.
For more detailed information, compile program and run 

$ cryptor -h

[ REQUIREMENTS ]
If you want to compile the project first, you will need openssl installed

If you do not want to compile this, the application is compiled in OUT.
To install without compilation:

$ cd OUT
$ chmod 777 *
$ ./install.sh

[ FOREWORD ]
I am not responsible for any damage you cause to anyone using this program.


[ TESTING ]
This program is frequently used by m and i work with lot of large files.
There was no data loss as I am using this program, only problem was when I forgot a password, then I have lost the file, because I could not decrypt it.

Currect version was tested in following way:
about 500 000 files with random sizes from 0 up to 2 GB were created, containing random data.
Then, they were encrypted and decrypted. All files were the same and no memoryleaks or errors

[ INSTALLATION ]

Program was written in NetBeans. You can eighter import it there, or

$ make

To clean, run

$ make clean


Then navigate to folder .OUT

$ cd OUT

here, run 

$ chmod +x ./install.sh
$ ./install.sh

This installs the program to the /usr/bin
and also creates autocomplete script in /etc/bash_completion.d

Then, you can open fresh terminal and type
$ cryptor -h

[ UNINSTALL ]
To uninstall, run

$ cd OUT
$ chmod +x *
$ ./uninstall.sh
