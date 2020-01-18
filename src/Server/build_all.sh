#!/usr/bin/env bash
NAME_API=weatherstation_api:latest
NAME_MANAGER=database_manager:latest

pushd ./api
docker rmi $NAME_API
docker build -t $NAME_API .
popd

pushd ./database_manager
docker rmi $NAME_MANAGER
docker build -t $NAME_MANAGER .
popd
