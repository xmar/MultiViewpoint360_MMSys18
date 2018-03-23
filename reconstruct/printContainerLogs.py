#!/usr/bin/env python3
import docker
import sys

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Wrong number of arguments')
        exit(1)

    dockerId = sys.argv[1]
    client = docker.from_env()
    c = client.containers.get(dockerId)
    for line in c.logs(stream=True):
        print(line.decode('utf-8'), end='')
