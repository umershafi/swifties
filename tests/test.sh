#!/bin/bash
printf "GET /static/hello.txt HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n"                    \
    | nc 127.0.0.1 8080

