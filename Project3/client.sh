#!/bin/bash
make -f Client clean
make -f Client
./Project3Client -h 127.0.0.1 -p 30950 -t 10 -i 4 -d mathcs10
