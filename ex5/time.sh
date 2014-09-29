#!/bin/bash

##
# Use `time` to measure the running times of dolist, dosync, and handoff
##

echo "Timing dolist..."
time ./dolist
echo -e "\nDone.\n"

echo "Timing dosync..."
time ./dosync
echo -e "\nDone.\n"

echo "Timing handoff..."
time ./handoff
echo -e "\nDone.\n"
