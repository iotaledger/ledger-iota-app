#!/bin/bash
git config --get remote.origin.url | tr -d '\n' > repository.url
docker build -t build-app .
