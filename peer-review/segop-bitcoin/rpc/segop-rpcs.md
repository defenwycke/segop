# segOP RPCs

RPCs to build, broadcast, and inspect segOP transactions.

## 1. createsegoptx

Builds a raw tx including segOP payload.

Example:

```
bitcoin-cli -regtest createsegoptx
'[{"txid":"TXID","vout":0}]'
'{"bcrt1qdestination...":0.001}'
'{"segop_hex":"00112233aabbcc..."}'
```

Returns hex-encoded raw tx (unsigned).

Options:

`segop_hex` – raw payload

`segop_tlvs` – TLV array (if supported)

## 2. decodesegoptx

Decode tx and show segOP data.

Example:

```
bitcoin-cli -regtest decodesegoptx "0200000001..."
```

Returns:

```
{
"txid":"...",
"segop":{
"size_bytes":128,
"weight":512,
"tlvs":[{"type":1,"length":32},{"type":2,"length":64}]
}
}
```

### 3. getsegopinfo

Summarise segOP metadata for a tx.

Example:

```
bitcoin-cli -regtest getsegopinfo "TXID"
```

Returns:

```
{
"has_segop":true,
"segop_bytes":128,
"segop_weight":512,
"segop_commitment":"segop1xyz...",
"tlv_summary":{"types":[1,2],"total_records":2}
}
```

### 4. getblocksegopstats

Report segOP usage per block.

Example:

```
bitcoin-cli -regtest getblocksegopstats "BLOCKHASH"
```

Returns:

```
{
"blockhash":"...",
"height":123,
"tx_count":10,
"segop_tx_count":3,
"segop_total_bytes":5120,
"segop_share_of_block_weight":0.18
}
```

### 5. Helper scripts

See `peer-review/segop-bitcoin/scripts/`:

`segop_regtest_demo.sh` – mines blocks, sends demo segOP tx.

`segop_send_example.py` – JSON-RPC automation for testing.

### 6. Compatibility

Additive RPCs – no change to legacy behaviour.
Optional use for segOP-aware nodes and tools.
