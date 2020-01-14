#!/usr/bin/env bash
NAME=database_manager:latest

docker rmi $NAME
docker build -t $NAME .
