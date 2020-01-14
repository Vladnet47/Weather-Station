#!/usr/bin/env bash
NAME=weatherstation_api:latest

docker rmi $NAME
docker build -t $NAME .
