#!/bin/bash
SCRIPT_DIR=$(dirname "$0")
if [ $SCRIPT_DIR == "./docker" ]; then
    DOCKERFILE_PATH="docker/Dockerfile"
    CONTEXT_PATH="."
else
    DOCKERFILE_PATH="Dockerfile"
    CONTEXT_PATH=".."
fi
docker build -t test-app $CONTEXT_PATH -f $DOCKERFILE_PATH
