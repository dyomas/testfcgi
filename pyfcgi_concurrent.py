#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket
import os;
import time;
import getopt;
import sys;
import pyfcgi

host = "localhost"
port = 1024
query = 'padding10dummy'
stdin = 'Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.'
dump = 0

def run():
  global query, host, port, stdin, dump

  s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

  print "===", host, port
  s.connect((host, port))

  b = pyfcgi.SocketBuffer()

  r = pyfcgi.RequestBegin(1)
  b.push(r)

  r = pyfcgi.RequestParams(1)
  r['QUERY_STRING'] = query
  b.push(r)

  r = pyfcgi.RequestStdin(1)
  r.data = stdin
  b.push(r)

  if dump:
    b.dump_output()
  b.write(s.fileno())

  time.sleep(3)
  b.read(s.fileno())
  if dump:
    b.dump_input()

  while True:
    a = b.pull()
    if a == None:
      break
    else:
      print a.type_name, '#', a.id
    if type (a) == pyfcgi.responseData:
      print a.data
    elif type (a) == pyfcgi.responseParams:
      for i in a:
        print "  ",  i.key, '=', i.value
    elif type (a) == pyfcgi.responseEnd:
      print "  appStatus =", a.appStatus
      print "  protocolStatus =", a.protocolStatus


def usage():
  print """Sent FastCGI queries
Options are:
  -Q (--query)         - QUERY_STRING, default `%(query)s`
  -H (--host)          - port to connect, default `%(host)s`
  -P (--port)          - port to connect, default `%(port)i`
  -S (--stdin)         - text to send as stdin, default `Lorem ipsum` classic placeholder
  -d (--dump)          - dump raw data
  -h (--help)          - show help message and exit""" %{"query":query, "host": host, "port": port}


def main():
  global query, host, port, stdin, dump

  params, args = getopt.getopt(sys.argv[1:], "Q:H:P:S:dh", ["query=", "host=", "port=", "stdin=", "dump", "help"])

  for opt, arg in params:
    if opt in ("-h", "--help"):
      usage()
      sys.exit()
    elif opt in ("-Q", "--query"):
      query = arg
    elif opt in ("-H", "--host"):
      host = arg
    elif opt in ("-P", "--port"):
      port = int(arg)
    elif opt in ("-S", "--stdin"):
      stdin = arg
    elif opt in ("-d", "--dump"):
      dump = 1

main()
run()
