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
 - Implemented new RPC `segopbuildp2sop` to derive the P2SOP OP_RETURN commitment from an arbitrary segOP payload:
  - Input: raw segOP payload hex (e.g. `01020304`)
  - Output: `534f50 || SHA256(payload)` for use in `{"data":"..."}` outputs.
- Defined a wallet-driven segOP transaction pipeline on branch `segop-v2`:
  - `segopbuildp2sop` → `createrawtransaction` → `fundrawtransaction` → `signrawtransactionwithwallet` → `createsegoptx` → `sendrawtransaction`.
- Successfully constructed a fully signed, segOP-bearing transaction via RPC only (no manual hex editing):
  - Included:
    - P2WPKH payment output to `bcrt1qm7nmrglev9qt0fqvk8df9e7qjvdmvmgeectxhu`.
    - A 0-value P2SOP output: `OP_RETURN 534f50‖SHA256(01020304)`.
    - Attached segOP payload: `01020304` (version 1).
- Broadcast and mined the transaction into a regtest block:
    ```
    `txid`: `b870015e786f8fa1bd6d4c49cc358a818f3ffe7020f6c6bc8c6d98ddf852e353`
    `getrawtransaction` (with blockhash) returns:
    `vout[2]` P2SOP OP_RETURN with the expected 35-byte commitment.
    `segop` object showing `version: 1`, `size: 4`, `hex: "01020304"`
    ```
- Confirmed the segOP consensus rules behave as designed:
  - Transaction is rejected if P2SOP is missing or mismatched (`bad-txns-segop-no-p2sop`, `bad-txns-segop-commitment-mismatch`).
  - With correct `segopbuildp2sop` output, transaction passes structural validation and is mined without issue.
- Outcome: **segOP v2 now supports automated, wallet-backed transaction creation** — the entire flow from “payload hex” to “mined segOP transaction” is expressible in a few RPC calls or a single wrapper script.
- - Introduced fully automated **segOP send workflow** via helper script `segop_send.sh`.
- Script now performs full lifecycle without manual input:
  1. Auto-loads wallet if not loaded.
  2. Builds `P2SOP` commitment from payload (SHA256 + SOP prefix).
  3. Constructs base transaction with both payment and P2SOP outputs.
  4. Funds and signs using wallet UTXOs (fee estimation via Core).
  5. Attaches segOP TLV payload using `createsegoptx` RPC.
  6. Broadcasts final segOP transaction to network.
  7. Optionally mines a confirming block and retrieves transaction state.
- Implemented auto-inspection output:
  - Prints concise `decoderawtransaction` with `segop` section.
  - Prints confirmed `getrawtransaction` view when mined.
- Example successful execution: ./segop_send.sh segoptest bcrt1qg3asjp6yvw2hwxe4e76exd52e7wlhv4r2tu5y0 0.1 01020304 mine

```
- Resulting transaction (TXID `11e92311ac3f4d8f8dee7fad7902c7542241333bbc0b5d2b94078a53b84423df`)
shows valid segOP structure:
"segop": {
"version": 1,
"size": 4,
"hex": "01020304"
}
```

- Verified end-to-end flow:
- segOP commitment generated correctly.
- Payload integrity maintained.
- segOP field visible and decoded post-block inclusion.
- Legacy node compatibility preserved.
- Next milestone: elevate `segop_send.sh` logic into an internal RPC (`segopsend`) for direct Core-level usage.

### 07-11-2025
- Completed integration of the segOP automated send workflow (segop_send.sh).
- Verified full transaction lifecycle.
  - Wallet auto-load (segoptest).
  - P2SOP commitment generation (534f50 || SHA256(payload)).
  - Transaction creation with standard output + 0-value P2SOP.
  - Funding, signing, and payload attachment using createsegoptx.
  - Broadcast and optional block mining.
- Confirmed script performs clean end-to-end flow with deterministic output.
- Verified payload integrity and correct segOP commitment inside mined blocks.
- Tested multiple payloads successfully — all mined, decoded, and verified via getrawtransaction.
- Confirmed legacy node compatibility — transactions accepted, segOP ignored safely.
- Began outlining design for internal RPC equivalent (segopsend) to replace external script.

### 08-11-2025
- Implemented and tested segopsend RPC command, a native Core replacement for the shell script.
  - Automates full workflow: builds P2SOP commitment, funds, signs, attaches segOP payload, and broadcasts.
  - Optional "mine" flag triggers local block generation for confirmation.
  - Verified returned TXID and payload integrity match those from segop_send.sh.
- Added decodesegoptx RPC to extract and display segOP TLV data directly from a raw transaction or TXID.
  - Outputs decoded version, length, payload hex, and text fields.
  - Used to verify block-embedded payloads without manual hex parsing.
- Deployed basic web wallet interface for local testing (React + simple JSON-RPC bridge):
  - Allows users to enter payloads, send segOP transactions, and view decoded data.
  - Connects directly to local Core node via RPC for regtest demonstration.
  - Confirmed successful end-to-end flow: payload → segopsend → mined → decodesegoptx → UI display.
  - Confirmed stability under multiple sequential transactions and reboots.
- segOP now fully functional from RPC to web interface — complete regtest demonstration stack achieved (Core, RPC, and UI layers unified).
