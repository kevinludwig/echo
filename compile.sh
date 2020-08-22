#!/bin/bash

gcc -o echoclient echoclient.c
gcc -o echoserver_single_threaded echoserver_single_threaded.c
gcc -o echoserver_multi_tbreaded -lpthread echoserver_multi_threaded.c
gcc -o echoserver_select echoserver_select.c
# gcc -o echoserver_uv -luv echoserver_uv.c


