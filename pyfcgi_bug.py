#!/usr/bin/python
# -*- coding: utf-8 -*-

# Пример моделирует проявление бага
#   `read` failed, 104, Connection reset by peer
# Оказывается, что при таком раскладе входящее сообщение прилетает
# в аккурат без FCGI_STDIN (если тот пустой, то это ровно 8 байт)
# А, поскольку парсер FCGI не считает (точнее не считал) cin обязательным,
# то он на них забивал и на момент закрытия сокета эти несчастные байты
# оставались непрочитанными. Откуда и проблема...


import socket
import pyfcgi

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 1024))

b = pyfcgi.SocketBuffer()

r = pyfcgi.RequestBegin(1)
b.push(r)

r = pyfcgi.RequestParams(1)
r['QUERY_STRING'] = """%D1%81%D0%BD%D0%BE%D0%B2%D0%B0+%D0%B7%D0%B4%D0%BE%D1%80%D0%BE%D0%B2%D0%B0"""
r['REQUEST_METHOD'] = 'GET'
r['CONTENT_TYPE'] = ''
r['CONTENT_LENGTH'] = ''
r['SCRIPT_NAME'] = '/q.exe'
r['REQUEST_URI'] = """/q.exe?%D1%81%D0%BD%D0%BE%D0%B2%D0%B0+%D0%B7%D0%B4%D0%BE%D1%80%D0%BE%D0%B2%D0%B0"""
r['DOCUMENT_URI'] = '/q.exe'
r['DOCUMENT_ROOT'] = '/home/saltaev/VCS/v256ru/trunk/tmp/download'
r['SERVER_PROTOCOL'] = 'HTTP/1.1'
r['GATEWAY_INTERFACE'] = 'CGI/1.1'
r['SERVER_SOFTWARE'] = 'nginx/1.5.8'
r['REMOTE_ADDR'] = '127.0.0.1'
r['REMOTE_PORT'] = '44524'
r['SERVER_ADDR'] = '127.0.0.1'
r['SERVER_PORT'] = '80'
r['SERVER_NAME'] = 'localhost'
r['REDIRECT_STATUS'] = '200'
r['SCRIPT_FILENAME'] = '/home/saltaev/VCS/v256ru/trunk/tmp/download/q.exe'
r['HTTP_HOST'] = 'localhost'
r['HTTP_CONNECTION'] = 'keep-alive'
r['HTTP_ACCEPT'] = 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8'
r['HTTP_USER_AGENT'] = 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36'
r['HTTP_ACCEPT_ENCODING'] = 'gzip,deflate,sdch'
r['HTTP_ACCEPT_LANGUAGE'] = 'en-US,en;q=0.8,ru;q=0.6'
r['HTTP_COOKIE'] = 'my_wikiUserID=1; my_wikiUserName=Dementiy+P.+Saltaev'


b.push(r)

r = pyfcgi.RequestStdin(1)
b.push(r)

b.dump_output()
b.write(s.fileno())

b.read(s.fileno())
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
