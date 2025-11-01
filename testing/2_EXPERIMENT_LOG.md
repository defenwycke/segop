# segOP Experiment Log #2  
**Date:** 1 November 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)

## Objective
Continue from Experiment Log #1 by embedding segOP support directly into Bitcoin Core (v30), enabling native creation, serialization, and decoding of segOP payloads through RPC.

## Summary of Progress

### 1. segOP Integration into Core
- Implemented new **`src/segop/`** module with:
  - `segop.h` — main TLV structure and serialization definitions  
  - `serialize.cpp` — TLV encoder/decoder logic  
  - `marker.cpp` — binary builder for SG-prefixed lane sections
 
- Added `std::optional<segop::SegopPayload>` field to `CTransaction` for direct transaction-level attachment.

- Updated **`miner.cpp`** to embed a segOP payload in each coinbase transaction:
  ```cpp
  auto seg = segop::MakeTestPayload("segOP: mined on regtest");
  coinbaseTx.segop_payload = std::move(seg);

### 2. RPC Layer (segOP Interface)
New RPC commands introduced in rpc_segop.cpp:

- `addsegoptx` "text"	Builds a segOP TLV section from a given text message
- `decodesegop` "hex"	Parses a segOP section back into structured JSON
- `getsegopdata` "blockhash"	(Experimental) Fetches segOP data from a block; currently returns dummy payload

These RPCs were compiled and successfully exposed through Core’s help system after fixing RPCHelpMan newline assertion issues.

### 3. Successful Encode → Decode Round Trip
First verified segOP payload transmitted and decoded via RPC:

Command sequence:

./bitcoin-cli addsegoptx "Hello from Defenwycke / segOP"
./bitcoin-cli decodesegop "5347011c01011948656c6c6f2066726f6d2043616c6c756d202f207365674f50"
Result:

```json

{
  "kind": "section",
  "segop": {
    "magic": "0x47504553",
    "version": 1,
    "fields": [
      {
        "type": 1,
        "len": 25,
        "value_text": "Hello from Defenwycke / segOP"
      }
    ]
  }
}
```
* This confirms full encode/decode parity — the first functional segOP message. // Name changed for anon

### 4. Observations
- segOP serialization is stable and deterministic across calls.
- RPC layer now safely handles all payload sizes within TLV limits.
- Coinbase embedding verified via miner instrumentation (regtest-only).
- Next step: connect getsegopdata to real block reads.

