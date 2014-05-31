#!/bin/sh

readonly ROOT_DIR=`dirname $0`
readonly COUNTER=${1:-10}

cd $ROOT_DIR

./pyfcgi_concurrent.py --query "padding${COUNTER}<*******>" > __1.stdout &
./pyfcgi_concurrent.py --query "padding${COUNTER}<------->" > __2.stdout  &
./pyfcgi_concurrent.py --query "padding${COUNTER}<=======>" > __3.stdout  &


