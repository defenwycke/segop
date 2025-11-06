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

### 02-11-2025
- Attempted direct integration of segOP hooks into Bitcoin Core v30 (transaction.h, merkle.cpp, rpc/rawtransaction.cpp).
- Initial compile succeeded, but introduced circular includes and header dependency conflicts when referencing segOP types.
- Encountered unresolved linker symbols for segop::DecodeTLV and RPC build regression in libbitcoin_common.a.
- Realized direct modification of Core created merge and traceability issues — code contamination too high for maintenance.
- Decision made to rebuild segOP as a fully modular extension, not a deep Core patch.
- Defined code-marking standard for traceability:
```
Multi-line blocks wrapped with //segOP code start>>> and //segOP code end<<<
Single-line edits tagged with //segOP
```
- Created plan for new clean branch feature/segop_clean to isolate development.
- All segOP modules (TLV, encoder/decoder, markers) to remain under src/segop/ with minimal Core-facing hooks.
- RPC interface to be rebuilt as an external registration module (rpc_segop.cpp) rather than internal patching.
- Aim: Clean rebuild → clear separation → traceable diffs → upstream-safe.
- Current status: preparing fresh v30 clone for clean reimplementation.

### 03-11-2025
- Began reconstruction of segOP integration using a **backward-compatible SegWit-style design**.  
- Modified `CMutableTransaction` to include a new `CSegOP m_segop` field for TLV data while preserving legacy `vin`/`vout` structure.  
- Extended `CTransaction` (immutable class) with a const `m_segop` field, fully synchronized with the mutable version during construction.  
- Patched both constructors in `transaction.cpp` to copy/move segOP payloads correctly and maintain deterministic hashing.  
- Ensured field ordering and const-safety — segOP data is now bound to every transaction object without altering hash computation yet.  
- Successfully compiled full Core build (`bitcoind`, `bitcoin-cli`, `test_bitcoin`) — **first clean segOP-aware build**.  
- Verified node start-up and RPC connectivity on regtest — all legacy behavior intact.  
- Prepared for serialization layer integration to follow SegWit’s marker/flag pattern:  
  - `(0x00 0x01)` → SegWit only  
  - `(0x00 0x02)` → segOP only  
  - `(0x00 0x03)` → combined SegWit + segOP  
- Next stage: implement segOP marker/flag in `SerializeTransaction()` and `UnserializeTransaction()`, followed by new `getsegopdata` RPC rebuild.  
- **Milestone achieved:** segOP structure embedded into Core transaction model, build verified, runtime stable.

### 04-11-2025
- Implemented segOP serialization layer within `SerializeTransaction()` and `UnserializeTransaction()`.  
- Defined new marker–flag scheme:  
  - `0x00 0x01` = SegWit only  
  - `0x00 0x02` = segOP only  
  - `0x00 0x03` = combined SegWit + segOP  
- Added `TX_WITH_SEGOP` and `TX_WITH_WITNESS_AND_SEGOP` macros mirroring existing SegWit logic.  
- Extended deserializer with safe fall-through detection for legacy nodes.  
- Updated `core_read.cpp` and `core_write.cpp` to correctly recognize segOP marker bytes.  
- Confirmed backward-compatibility — legacy node decodes standard transactions unchanged.  
- segOP-aware node now cleanly serializes TLV payloads appended after witness data.  
- **Milestone:** first fully serialized segOP transaction accepted by local decoder.

### 05-11-2025
- Built and verified RPC command `createsegoptx` inside `rpc/rawtransaction.cpp`.  
- Command allows payload injection into any signed raw transaction:  
  `createsegoptx "hexstring" "payload" → {"hex": "...segop..."}`
- RPC help text, examples, and argument checks completed.  
- Added consensus validation in `tx_check.cpp` requiring:  
  - ≤ 100 KB segOP payload,  
  - exactly one P2SOP output,  
  - and SHA256(payload) = commitment in that output.  
- Introduced new policy errors:  
  `bad-txns-segop-no-p2sop` and `bad-txns-segop-commitment-mismatch`.  
- Compiled full Core build successfully with RPC + consensus hooks active.  
- segOP decoding verified via `decoderawtransaction` → correct `"segop"` field exposed.  
- Environment snapshot archived — **segOP now RPC-accessible and consensus-enforced.**

### 06-11-2025
- Launched regtest environment with wallet `segoptest` to validate full segOP workflow.  
- Constructed raw transaction containing proper P2SOP output (`OP_RETURN 0x23 "SOP" <32 B hash>`).  
- Used `createsegoptx` to append payload `01020304` producing final serialized transaction.  
- Verified round-trip decode:
  ```json
  "segop": { "version": 1, "size": 4, "hex": "01020304" }
  ```
- Broadcast transaction → rejected until P2SOP added, proving consensus check active.
- Added correct P2SOP → mempool accept succeeded.
- Mined transaction into regtest block #105.
- Retrieved via getrawtransaction TXID 1 <blockhash> showing confirmed segOP + matching SOP commitment.
- Confirmed backward-compatibility: legacy decode unaffected, segOP node displays new field.
- Result: first fully mined segOP transaction in Bitcoin Core v30 (regtest).
- All components—serialization, RPC, consensus, and mining—verified end-to-end.
