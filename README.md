# Netstore
Server - Client program for UDP/TCP polling and sending/uploading files

## Using:
* C++ 11
* Boost.Program_options
* Boost.Algorithm

## Compile
`make server && make client`

## Run
`./server -g [multicast address] -p [port] -f [dir with kept files] -b [optional: max space] -t [optional: timeout of waiting for client to connect]`

`/client -g [multicast address] -p [port] -o [downloaded files dir] -t [optional: timeout of waiting for servers responses]`

## Example
`./server -g 224.0.2.1 -p 12345 -f example/`

`./client -g 224.0.2.1 -p 12345 -o download/`
