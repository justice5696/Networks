#!/bin/bash
make -f Server clean
make -f Server
./Project3Server -p 30950 -d database.dat
