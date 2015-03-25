#!/bin/sh
#
# Usage: ./verify.sh pyinotify-x.x.x.tar.gz

# Get my (seb@dbzteam.org) public signing key from pgp.mit.edu
gpg --keyserver pgp.mit.edu --recv-keys E8440341

# Verify the signature
gpg --verify $1.asc
if [ $? -eq 0 ]
then
    echo "\033[32m[OK    ] signature\033[0m"
else
    echo "\033[31m[FAILED] signature\033[0m"
fi

# Verify the checksum
sha256sum -c $1.sha256
if [ $? -eq 0 ] 
then
    echo "\033[32m[OK    ] checksum\033[0m"
else
    echo "\033[31m[FAILED] checksum\033[0m"
fi
