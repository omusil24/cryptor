# cryptor 2.2

This project was created by Jan Glaser.

You can freely copy or redistribute the program, but provide there my name, as author, please.

The main idea of the program is to allow safe encryption / decryption of files.
For more detailed information, compile program and run 
```
cryptor -h
```

## Requirements

If you want to compile the project first, you will need openssl installed

If you do not want to compile this, the application is compiled in OUT.
To install without compilation:
```
cd OUT
chmod 755 *
./install.sh
```

## Foreword

I am not responsible for any damage you cause to anyone using this program.

## Testing

This program is frequently used by m and i work with lot of large files.
There was no data loss as I am using this program, only problem was when I forgot a password, then I have lost the file, because I could not decrypt it.

Currect version was tested in following way:
about 500 000 files with random sizes from 0 up to 2 GB were created, containing random data.
Then, they were encrypted and decrypted. All files were the same and no memoryleaks or errors

## Installation

Program was written in NetBeans. You can either import it there, or run
```
make clean
make
```

Then navigate to folder `OUT` and execute following:
```
chmod +x ./install.sh
./install.sh
```

This installs the program to `/usr/bin` directory and also creates autocomplete script in `/etc/bash_completion.d`.

Then, you can open fresh terminal and type
```
cryptor -h
```

## Uninstall

To uninstall, run
```
cd OUT
chmod +x *
./uninstall.sh
```
