#!/bin/bash

cd common/ && make clean && make && cd ..
cd msg/ && make clean && make && cd ..
cd queue/  && make clean && make && cd ..
cd socket/  && make clean && make && cd ..
cd engine/  && make clean && make && cd ..

