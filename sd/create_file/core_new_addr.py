#!/usr/bin/env pyhton3

import json
import argparse
from bitcoin_rpc_class import RPCHost


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-r', '--rpc-url', help='Bitcoin node URL, eg http://USERNAME:PASSWORD@HOST:PORT/wallet/WALLET', required=True)
    parser.add_argument('-n', '--network', help='Bitcoin network (mainnet, testnet)', required=True)
    parser.add_argument('-a', '--amount',  help='Number of new addresses', required=True)
    args = parser.parse_args()

    rpc = RPCHost(args.rpc_url) # TODO: add checks

    res = {}
    res['network'] = args.network # TODO: add checks
    res['addresses'] = []

    for i in range(1, int(args.amount)):
        address = rpc.call('getnewaddress')
        res['addresses'].append(address)

    print(json.dumps(res, indent=4))


if __name__ == '__main__':
    main()
