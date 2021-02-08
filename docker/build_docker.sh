#!/bin/bash

#[ ! -f "git.priv" ] && {
#    echo "copy your gitlab private key into this directory with filename 'git.priv'"
#    exit 1
#}

docker build -t ledger_iota .
