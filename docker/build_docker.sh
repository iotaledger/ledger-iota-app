#!/bin/bash

#[ ! -f "git.priv" ] && {
#    echo "copy your gitlab private key into this directory with filename 'git.priv'"
#    exit 1
#}

git config --get remote.origin.url | tr -d '\n' > repository.url

docker build -t ledger_iota .
