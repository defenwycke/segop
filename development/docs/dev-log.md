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

### 09-11-2025
- Continued refinement of segOP implementation and documentation.
- Drafted Experiment Log Zero summarizing the full segOP design, rationale, and implementation goals.
- Updated experiment spec to clearly define:
  - segOP as a fully-fee-paying structured data lane following the SegWit model.
  - Objectives: integrate, expose RPCs, validate backward compatibility, demonstrate pruning potential.
  - Verified continued stability of segopsend RPC under repeated regtest mining cycles.
  - Began preparations for segOP wallet integration with front-end React dashboard (segop-wallet / segop-ui projects).
  - Implemented live RPC connectivity test (getblockchaininfo, getnetworkinfo) to populate UI metrics.
  - Outlined component structure for app:
    - Views: Dashboard / Send / Inspector / Payloads / Simulator / Settings.
  - Types defined for NodeInfo, Settings, and PayloadTemplate.
  - Deployed local version of app.tsx connecting to backend RPC for data inspection.
  - Verified regtest node responds to UI payload creation and TX submission requests.
- Continued internal review of segOP consensus design for P2SOP enforcement and pruning logic.

### 10-11-2025
- Shifted focus to BIP preparation and peer review phase.
- Engaged with Bitcoin-dev mailing list — received first response from moonsettler requesting clarification on:
  - Node synchronization behavior between segOP-aware and legacy nodes.
  - Retention and pruning policies.
  - Drafted technical reply explaining:
    - segOP payloads are cryptographically committed but prunable post-validation.
    - Legacy nodes ignore segOP data; segOP-aware nodes validate then may discard payload bytes.
    - Distinction between full, partial, and pruned node policies.
    - OP_RETURN data cannot be pruned, whereas segOP enables verifiable pruning.
- Added clarification to proposal: segOP introduces a quarantine lane for arbitrary data, preventing spam and abuse of witness or OP_RETURN fields.
- Confirmed continued regtest node stability with mixed SegWit + segOP transactions.

### 11-11-2025
- Began cleanup and consolidation for segOP documentation set:
- Regenerated main repo README — concise, public-facing summary of segOP’s purpose, motivation, and parallels to SegWit.
- Added comparison table explaining how segOP restores fee fairness, enables pruning, and opens structured use cases (quantum sigs, rollup roots, vault metadata).
- Completed segOP Extended Transaction Specification (draft-1) in Markdown format:
  - Introduced clear section numbering, field tables, and examples for marker, flag, and TLV structure.
  - Defined new identifiers: txid, wtxid, and fullxid (extended ID including segOP).
  - Enforced single P2SOP per transaction consensus rule.
  - Clarified IBD (Initial Block Download) behavior — pruned payloads do not affect chain validity.
  - Performed cross-check of all technical sections for accuracy; corrected formatting and terminology inconsistencies.
  - Draft now ready for submission as Pre-BIP.
- Parallel front-end work:
  - Added createAddress and test utility functions to wallet UI for full segOP transaction flow demonstration.
  - Confirmed segopsend and decodesegoptx integrate correctly with web interface via local backend.
  - Conducted final validation run on regtest:
  - Generated and mined new segOP transaction using updated build.

### 12-11-2025
- After rebuilding the spec sheet - Code updates planned and started.
- Just receieved email from Moonsettler regarding pruning and p2p availability of segOP payload. Intrusive thoughts have now disrupted work flow.
- Developed new method of pruning and p2p payload verification that doesnt break relays.
- I will continue with the initial code updates to the spec. Build a new spec for pruning and integrate.
- It is a better method of pruning - so i am grateful for Moonsettlers questions.

### 13-11-2025
- Continued analysis of segOP pruning model following Moonsettler’s feedback.
- Verified that current segOP design still requires nodes to temporarily fetch payloads before validation, confirming pruning cannot occur before validation.
- Re-evaluated segOP relay behaviour to ensure nodes do not prune payloads prematurely; confirmed that the existing 24-block validation window in the spec works conceptually but needs clearer documentation.
- Double-checked OP_RETURN behaviour in Bitcoin Core: confirmed that OP_RETURN outputs cannot be pruned by nodes — their bytes remain permanently embedded in the scriptPubKey.
- Updated internal notes contrasting OP_RETURN (non-prunable) vs segOP (prunable after validation with retained commitment).
- Drafted improved explanation for mailing list reply:
  - legacy nodes validate and relay blocks without seeing segOP payloads
  - segOP nodes validate payload → relay → optionally prune post-validation
  - segOP payload commitment remains sufficient for future consensus checks
  - Reviewed segOP consensus flow again to ensure no contradictions with Bitcoin P2P rules.
- Confirmed regtest behaviour unchanged during pruning-model review; node stability remains intact.
- Documented pruning-model insights for integration into revised spec and BIP draft.

### 14-11-2025
- Reviewed segOP transaction pipeline diagram and identified formatting/clarity issues.
- Began drafting Appendix G (Worked Validation Example) and Appendix H (Rationale / Design Motivations).
- Wrote the initial full rationale for why segOP exists, including:
  - Bitcoin’s ongoing spam/ARB-data pressure
  -  Need for a structured, full-fee data lane
  - Prunability benefits over OP_RETURN
  - Preservation of node autonomy
  - Why arbitrary data is required for Bitcoin to act as a settlement and timestamping layer
- Drafted consolidated section: “What segOP could do for Bitcoin in its spam war”.
- Added early notes on how segOP enables future structured use cases (Qsig, rollups, commitments, vault metadata).
- Integrated rationale and spam-war content into the spec’s summary sections (no code changes).
- Documentation preparation only; no modifications to Core code.
- Wallet app.tsx redesign. Now includes dual wallet and shared mempool explorer. Enabling user to test from walet to wallet via an easy UI.

### 15-11-2025
- Started shunting segOP core v30 code to the latest spec.
- Implemented fullxid (extended transaction ID):
  - Added calculation path for the new fullxid, which includes the segOP data lane in the hash.
  - Ensured legacy txid and wtxid remain unchanged.
  - Confirmed code produces all three IDs deterministically.
  - Verified output via RPC inspection and internal logging.

### 16-11-2025
- Implemented strict TLV structure for segOP payloads.
- Added mandatory TLV parsing and encoding rules as defined in draft-1.
- Updated segOP serializer/deserializer to enforce:
  - `[type][length][value]` ordering
  - Proper length validation
  - Rejection of malformed TLVs
- Verified TLV round-trip stability through:
  - segopsend
  - decoderawtransaction
  - decodesegoptx
- Confirmed spec matches code behaviour exactly.

### 17-11-2025
- Completed checklist alignment for segOP implementation (draft-1 vs live code).
- Confirmed that two major requirements from the compliance checklist are already complete in Core:
  - fullxid (Extended Transaction ID) — COMPLETE
    - Verified fullxid calculation path matches draft-1.
    - Confirmed hashing includes segOP lane while legacy txid and wtxid remain unchanged.
    - RPC and logging confirm deterministic output across repeated transactions.
  - TLV Structure Enforcement — COMPLETE
    - Code now enforces strict [type][length][value] ordering.
    - Malformed TLVs rejected during parsing.
    - Verified round-trip through segopsend, decoderawtransaction, and decodesegoptx.
    - Behaviour matches the updated spec exactly.
- Spec Update: New TLV Data Types — COMPLETE
  - Updated the segOP Extended Transaction Specification to define additional TLV types.
  - Added type identifiers and descriptions beyond the default 0x01 text TLV.
  - Updated examples, field tables, and normative language to reflect multi-TLV sequences.
  - Clarified how nodes MUST parse unknown types safely (ignore value; retain length discipline).
  - Spec and code now aligned on TLV extensibility model.
- Confirmed successful validation across regtest, wallet RPC, and UI.
