#!/bin/bash

cd insrv/ && make clean && make && cd ..
cd midsrv/ && make clean && make && cd ..
cd outsrv/  && make clean && make && cd ..
cd db_insrv/  && make clean && make && cd ..
cd db_midsrv/ && make clean && make && cd ..

