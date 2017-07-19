#!/bin/bash

install_intelli_sense()
{
  #install autocomplete
 
  INTELI_S="/etc/bash_completion.d"
  if [ ! -d "$INTELI_S" ]; then
    if [ "$1" = "-r" ]; then
      echo "Failed to install intellisense: directory $INTELI_S does not exist. Quit"
    else
      echo "Failed to remove intellisense: directory $INTELI_S does not exist. Quit"
    fi
    exit 1 
  fi


  if [ "$1" = "-r" ]; then
    if [ -f "$INTELI_S/cryptor" ] ; then
      #uninstall
      echo "AAAA"
      rm "$INTELI_S/cryptor"
      return 0
    fi
    return 0
  fi

  #create autocomplete file
#------------------------------------------------------
  chmod +x "$APPNAME"
  local OPTS=`"$APPNAME" --print-all-options`
#------------------------------------------------------
echo 'APP_NAME="cryptor"

_cryptor () 
{  
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
' > "$INTELI_S/cryptor"
echo "    opts=\"$OPTS\"" >> "$INTELI_S/cryptor"
        
echo '

    # By default, autocomplete with options
    if [[ ${cur} == -* ]] ; then
        COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
        return 0
    fi

    # Else, autocomplete with files and dirs
    COMPREPLY=( $(compgen -f -- ${cur}) )
    return 0    
}
complete -o filenames -F _cryptor "$APP_NAME"' >> "$INTELI_S/cryptor"
  
#------------------------------------------------------
}

APPNAME="cryptor"

#if [ ! -f installman ]; then
#  echo "installman does not exist"
#  exit 1
#fi

UNINSTALL=""

if [ "$1" = "-r" ]; then
  UNINSTALL="-r"
fi

#chmod +x installman

#./installman "$APPNAME".7

if [ "$UNINSTALL" = "-r" ]; then
  #./installman -r "$APPNAME".7
  rm -f /usr/bin/"$APPNAME"
  install_intelli_sense "$UNINSTALL"
  echo "Uninstalled"
  exit 0
fi

if [ ! -f "$APPNAME" ]; then
  echo "File $APPNAME does not exist"
  exit 1
fi
cp "$APPNAME" /usr/bin/

#install icon
#ICO_DIR="/usr/share/applications"
#if [ ! -d "$ICO_DIR" ]; then
#  echo "Directory $ICO_DIR does not exist. Can not install icon"
#  exit 1
#fi

#echo "[Desktop Entry]
#Type=Application
#Terminal=true
#Name=Tor Browser
#Icon=
#Exec=/root/.torbrowser/Tor\ Browser" > "$ICO_DIR/cryptor.desktop"

install_intelli_sense "$UNINSTALL"

echo "Installed"
