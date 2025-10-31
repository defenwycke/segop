# Dev Log
### 2025-10-31
- After having no response from peers (bitcoin dev group) for discussion regarding segOP technical doc, I have decided to simulate and test this thesis out myself.
- Spun server up and have *2 V30 nodes running in signet - Old node and segOP node.
- Completed RPC: getsegopdata builds and returns TLV payloads.
- Directory cleanup - segop/ : clean namespace (segop.h, serialize.cpp, marker.cpp).
- Transaction.h: Now aware of optional segop_payload.
- Deamon runs and exposes RPC endpoint.
- Wired segOP into tx serialisation
- simple test TLV payload received
  ```
  {
  "hex": "534701190101167365674f5020544c562074657374207061796c6f6164",
  "text": "segOP TLV test payload"
  }
  ```
 - Early testing is showing this could be possible. The next phase is to prove segOPS tx between nodes and extract payload.

