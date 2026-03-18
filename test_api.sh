#!/bin/bash
cd /home/javier/cpp/http/build
./bin/http_server > /dev/null 2>&1 &
PID=\$!
sleep 1
echo " --- POST ALUMNO ---\
curl
