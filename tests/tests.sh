#!/bin/sh

cd tests
cd BPF
BPF.sh $1
cd ../BPF_emu_shifts
BPF_emu_shifts.sh $1
cd ../BPX
BPX.sh $1
cd ../one-time-INT3
one-time-INT3.sh $1
cd ..
cd ..


