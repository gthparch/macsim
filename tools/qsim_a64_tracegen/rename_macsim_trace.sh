#!/bin/sh

# Use at your own risk!

if [ $# -ne 4 ]
then
    echo "Usage: $0 <old basename> <new basename> <start> <K>"
    echo "Change <old>-0.log.gz, ... <old>-K.log.gz, to <new>-0.log.gz, ... <new>-K.log.gz"
    echo "For example, to rename a-0.log.gz, a-1.log.gz to b-0.log.gz, b-1.log.gz:  rename.sh a b 0 2"
    exit 1
fi

num_trace_files=`expr $4 - 1`;
for i in `seq 0 $num_trace_files`
do
    num=`expr $3 + $i`;
    if [ -e ${1}-${num}.log.gz ]
    then
        mv ${1}-${num}.log.gz $2_$i.raw
    else
        echo "$1-$num does not exist!"
    fi
done
