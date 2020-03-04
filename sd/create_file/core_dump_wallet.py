#!/usr/bin/env pyhton3

import json
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--file', help='Bitcoin core wallet dump file', required=True)
    parser.add_argument('-n', '--network', help='Bitcoin network (mainnet, testnet)', required=True)
    args = parser.parse_args()

    res = {}
    res['network'] = args.network # TODO: add checks
    res['addresses'] = []

    fileHandler = open (args.file, "r")
    listOfLines = fileHandler.readlines()
    fileHandler.close()
    for line in listOfLines:
        elements = line.strip().split(' ')
        if len(elements) == 5:
            addr = elements[4].split('=')
            if len(addr) == 2:
                res['addresses'].append(addr[1])

    print(json.dumps(res, indent=4))


if __name__ == '__main__':
    main()
