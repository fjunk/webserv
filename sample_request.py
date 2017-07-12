#!/usr/bin/env python
# -*- coding: utf-8 -*- 

import requests 



s = ("GET /files/index.html HTTP/1.0\r\n"
 "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
 "Host: www.tutorialspoint.com\r\n"
 "Accept-Language: en-us\r\n"
 "Accept-Encoding: gzip, deflate\r\n"
 "Connection: Keep-Alive\r\n\r\n")

with open('input.txt', 'w') as f: 
    f.write(s)


