#!/bin/bash
rpath="$( dirname "$( readlink -f "$0")" )"
cd $rpath

mkdir output

# build docker container
docker build . -f Dockerfile -t codeql

# run analysis
docker run --rm -v $rpath/output:/app/output -v $rpath/..:/app/ledger-iota-app -it codeql "/app/codeql-local.sh"
