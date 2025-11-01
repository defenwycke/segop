# Dev Log

### 31-10-2025
- After no response from peers (bitcoin dev group) regarding segOP technical doc, decided to test and simulate the thesis independently.
- Spun up dedicated server with 2 Core v30 nodes — baseline node and segOP node.
- Configured regtest network for isolated testing.
- Created clean segOP namespace under `/src/segop/` (segop.h, serialize.cpp, marker.cpp).
- Added optional `segop_payload` field to transaction.h.
- Extended transaction serialization and deserialization to include segOP payloads.
- Integrated segOP into TX flow without disrupting legacy parsing.
- Compiled segOP-enabled daemon successfully.
- Added new RPC command `getsegopdata` to retrieve TLV payloads from segOP transactions.
- Confirmed RPC returns valid TLV hex and text data.
  ```json
  {
    "hex": "534701190101167365674f5020544c562074657374207061796c6f6164",
    "text": "segOP TLV test payload"
  }
  ```
- Verified TLV encoding and decoding path end-to-end.
- segOP transaction accepted and visible on regtest node.
- Legacy node ignores segOP payload cleanly (backward compatible).
- Confirmed segOP RPC output correct and deterministic.
- Validated TLV section appears after witness data in serialized transaction.
- Clean build and stable runtime confirmed under multiple reboots.
- Environment snapshot and configs saved for repeat testing.
- Early proof complete — segOP functional on regtest and coexists with non-segOP nodes.

### 01-11-2025
- Generated and broadcast first encoded segOP transaction on regtest network.
- Transaction hex (segOP embedded after witness):
```
010000000001010000000000000000000000000000000000000000000000000000000000000000ffffffff2503a30f1d04ffff001d010447534701190101167365674f5020544c562074657374207061796c6f616400000000
```
- Mined block containing segOP transaction using segOP node.
- Retrieved transaction via getsegopdata RPC and decoded TLV payload successfully.
- Decoded output verified as:
```
{
  "txid": "b40cd3e26ab72d8fa3b1c2c2e72ad623ec94e4c53b392d0e1a7bde0f833420aa",
  "segop": {
    "hex": "534701190101167365674f5020544c562074657374207061796c6f6164",
    "text": "segOP TLV test payload"
  }
}
```
- Verified full encode/decode round-trip: RPC → TX → Block → RPC.
- Confirmed payload integrity and deterministic reproduction.
- Legacy node parsed and ignored segOP data without error.
- Confirmed regtest block acceptance with segOP payload intact.
- Saved raw transaction and decoded output for archive reference.
- Proof-of-concept complete: segOP TLV data embedded, mined, and extracted successfully on regtest network.
