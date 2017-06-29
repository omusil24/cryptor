#!/bin/bash

APPNAME="cryptor"

if [ ! -f installman ]; then
  echo "installman does not exist"
  exit 1
fi

UNINSTALL=""

if [ "$1" = "-r" ]; then
  UNINSTALL="-r"
fi

chmod +x installman

./installman "$APPNAME".7

if [ "$UNINSTALL" = "-r" ]; then
  ./installman -r "$APPNAME".7
  rm /usr/bin/"$APPNAME"
  exit 0
fi

if [ ! -f "$APPNAME" ]; then
  echo "File $APPNAME does not exist"
  exit 1
fi
cp "$APPNAME" /usr/bin/

echo "Installed"
