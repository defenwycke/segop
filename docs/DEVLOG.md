# Dev Log
### 2025-10-31
- After having no response from peers for discussion RE segOP technical doc, I have decided to simulate and test this thesis out myself.
- Spun server up and have *2 V30 nodes running in signet - Old node and segOP node.
- Completed RPC: getsegopdata builds and returns TLV payloads.
- Directory cleanup - segop/ : clean namespace (segop.h, serialize.cpp, marker.cpp).
- Transaction.h: Now aware of optional segop_payload.
- Deamon runs and exposes RPC endpoint.
