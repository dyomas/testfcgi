#!/bin/sh

readonly ROOT_DIR=`dirname $0`
readonly MYNAME=`basename $0`
readonly HOST_DEFAULT=localhost
readonly PORT_DEFAULT=1024
readonly COUNTER_DEFAULT=15

EXIT=0
USAGE=0
VERBOSE=0
HOST=$HOST_DEFAULT
PORT=$PORT_DEFAULT
COUNTER=$COUNTER_DEFAULT

usage () {
  echo "Caller of same named Python scripts
Usage: $MYNAME [options]
Options are:
  -H - host, default \`$HOST_DEFAULT\`
  -P - port, default $PORT_DEFAULT
  -C - counter of dummy strings repetition, default $COUNTER_DEFAULT
  -v - show diagnostic messages
  -vv - show diagnostic messages and set -x
  -h - show help message and exit" >&2
}

while getopts H:P:C:vh Option; do
  case $Option in
    H) HOST=$OPTARG
    ;;
    P) PORT=$OPTARG
    ;;
    C) COUNTER=$OPTARG
    ;;
    v)
      VERBOSE=$((VERBOSE + 1))
      if [ $VERBOSE -gt 1 ];
      then
        VERBOSE_CHILD=-vv
      else
        VERBOSE_CHILD=-v
      fi
    ;;
    h) USAGE=1
    ;;
    *)
      USAGE=1
      EXIT=1
    ;;
  esac
done

if [ $USAGE -ne 0 ];
then
  usage
  exit $EXIT
fi

if [ $VERBOSE -ne 0 ];
then
  echo "HOST = \`${HOST}\`
PORT = ${PORT}
COUNTER = ${COUNTER}"
fi

if [ $VERBOSE -gt 1 ];
then
  set -x
fi

cd $ROOT_DIR

./pyfcgi_concurrent.py --host $HOST --port $PORT --query "padding${COUNTER}<*******>" > __1.stdout &
./pyfcgi_concurrent.py --host $HOST --port $PORT --query "padding${COUNTER}<------->" > __2.stdout &
./pyfcgi_concurrent.py --host $HOST --port $PORT --query "padding${COUNTER}<=======>" > __3.stdout &


