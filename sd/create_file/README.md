# Create BTC.TXT

## Using a wallet dump

Dump you wallet using the bitcoin-cli and call `core_dump_wallet.py` passing the generated file.

```
bitcoin-cli dumpwallet mywallet.txt
python3 core_dump_wallet.py  -f mywallet.txt -n testnet
```

## Generating new addresses
Use RPC in order to generate addresses using the script `core_new_addr.py`.

```
python3 core_new_addr.py -n testnet -a 10 -r http://username:password@127.0.0.1
```
