# segOP-Extended Transaction Specification

**Name:** segOP (Segregated OP_RETURN)  
**Author:** Defenwycke  
**Version:** draft-1  
**Date:** November 2025  
**Status:** Pre-BIP draft (normative spec)  

## 1. Overview

segOP (Segregated OP_RETURN) introduces a dedicated, prunable data section to Bitcoin transactions, positioned **after SegWit witness data** and **before nLockTime**, fully charged at `4 WU / byte`.

The segOP lane:

- Holds arbitrary or structured payloads (e.g. inscriptions, commitments, vault metadata, rollup roots).  
- Is logically separate from both `scriptSig` and witness data.  
- Is cryptographically bound to the transaction via a **P2SOP** (Pay-to-SegOP) commitment output.  
- Enables nodes to **prune payload bytes** after validation and sufficient block depth while preserving consensus validity.  
- Is soft-fork compatible and invisible to legacy nodes.

Legacy nodes compute the same `txid` and consider segOP-bearing transactions valid without parsing segOP. segOP-aware nodes enforce payload rules and may select their own pruning or archival storage policy.

## 2. Key Properties

**Post-witness data lane:**

`core tx → witness? → segOP? → nLockTime`

**Full-fee accounting:**
- Every segOP byte is charged at `4 weight units / byte` (no SegWit discount).

**Commitment binding via P2SOP:**
- A dedicated OP_RETURN output commits to a tagged SHA256 of the segOP payload:

`segop_commitment = TAGGED_HASH("segop:commitment", segop_payload)`

**Unified marker + flag signaling:**
  - `marker = 0x00`  
  - `flag` bitfield:  
    - `0x01` = SegWit present  
    - `0x02` = segOP present  
    - `0x04` and above = reserved

**Prunable payloads:**
- Nodes validate segOP payloads and commitments, then may prune segOP bytes, similar to block pruning today.  

**Soft-fork compatible:**
- `txid` and `wtxid` are unchanged; legacy nodes accept segOP transactions.

## 3. Transaction Wire Layout

Extended transactions appear on the wire as follows (little-endian where noted):

```
    [ nVersion (4 bytes, LE) ]

    [ marker (1) = 0x00 ]
    [ flag   (1) = bitfield {SegWit = 0x01, segOP = 0x02} ]

    [ vin_count (varint) ]
      repeat vin_count times:
        [ prevout_hash  (32, LE) ]
        [ prevout_index (4, LE) ]
        [ scriptSig_len (varint) ]
        [ scriptSig     (bytes) ]
        [ nSequence     (4, LE) ]

    [ vout_count (varint) ]
      repeat vout_count times:
        [ value             (8, LE) ]
        [ scriptPubKey_len  (varint) ]
        [ scriptPubKey      (bytes) ]

    if (flag & 0x01) // SegWit
      [ witness for each input ]

    if (flag & 0x02) // segOP
      [ segop_marker  = 0x53 ('S') ]
      [ segop_flag    = 0x01 ]        # segOP v1
      [ segop_len     (varint) ]
      [ segop_payload (segop_len bytes) ]

    [ nLockTime (4, LE) ]
```

SegWit and segOP can be present independently or together, as indicated by the `flag` bitfield.

## 4. Marker and Flag Definition

### 4.1 Marker and Flag

| Field  | Size (bytes) | Value / Type | Purpose                                   |
|--------|--------------|--------------|-------------------------------------------|
| marker | 1            | 0x00         | Signals "extended transaction" format     |
| flag   | 1            | bitfield     | Indicates presence of SegWit and/or segOP |

### 4.2 Flag Bits

| Bit | Hex  | Meaning        |
|-----|------|----------------|
| 0   | 0x01 | SegWit present |
| 1   | 0x02 | segOP present  |
| 2   | 0x04 | Reserved       |

### 4.3 Examples

| Flag | Binary     | Meaning         |
|------|------------|-----------------|
| 0x01 | 0000 0001  | SegWit only     |
| 0x02 | 0000 0010  | segOP only      |
| 0x03 | 0000 0011  | SegWit + segOP  |

For SegWit + segOP transactions, the bytes immediately following `nVersion` are:`00 03`.

### 4.4 Parsing Rules

**Legacy Compatibility**
- Legacy nodes interpret `marker = 0x00` as a zero-length `vin` and reject such transactions; upgraded nodes detect the `marker`–`flag` pair before normal deserialization.

**Extended Detection**
- If bit 0 is set (`flag & 0x01`), SegWit data follows `vout`.  
- If bit 1 is set (`flag & 0x02`), segOP data follows SegWit (if present) or directly follows `vout` (if SegWit absent).

**Pruning Behavior**
- Nodes that prune SegWit or segOP data may skip over these sections during block validation after verifying that Merkle roots and commitments match.

**Reserved Bits**
- Bits ≥ `0x04` are reserved and MUST be ignored by compliant parsers to allow future extensions without breaking compatibility.

## 5. segOP Section

A segOP section is appended after witness (if present) and before `nLockTime`:

```
    [ segop_marker  = 0x53 ]        # ASCII 'S'
    [ segop_flag    = 0x01 ]        # segOP v1
    [ segop_len     = <varint> ]    # length of payload in bytes
    [ segop_payload (segop_len bytes) ]
```

Consensus rules:

- For segOP v1, `segop_flag` MUST be `0x01`. Any other value is invalid.  
- `segop_len` MUST NOT exceed `MAX_SEGOP_TX_BYTES`.  
- The transaction MUST be rejected if the serialization does not contain exactly `segop_len` bytes of segOP payload.

## 5.2 TLV Payload (Mandatory Format)

Each segOP payload MUST be encoded as a sequence of TLV records:

    [type (1 byte)] [len (1 byte)] [value (len bytes)]

Where:

- `type`: 1-byte unsigned integer.  
- `len` : 1-byte unsigned integer, length of `value` in bytes.  
- `value`: `len` bytes of opaque data.

### 5.2.1 TLV Well-Formedness (Consensus)

Let `segop_len` be the total payload length.

Starting from offset `0`:

1. If `offset == segop_len`, stop (valid end).  
2. If `offset > segop_len`, invalid.  
3. Read `type` (1 byte).  
4. Read `len` (1 byte).  
5. If `offset + 2 + len > segop_len`, invalid (TLV overruns payload).  
6. Advance `offset += 2 + len` and repeat.

At the end of parsing, the transaction is valid **only if** `offset == segop_len`. Any trailing bytes, truncated TLV, or overflow MUST make the transaction invalid.

### 5.2.2 Type Semantics

- TLVs MAY appear in any order.  
- Duplicate `type` values are allowed.  
- Unknown `type` values are allowed and treated as opaque by consensus.  
- Higher-layer protocols MAY assign semantics to types. Future soft-forks MAY define special meaning for specified types; old nodes will continue to treat them as opaque data.

**Example segOP payload:**

    01 10 7365674f5020544c562074657374   # type=1, len=16, "segOP TLV test"
    02 04 00000001                       # type=2, len=4, app-specific value

## 6. Worked Example (SegWit + segOP, flag = 0x03)

Illustrative only (not a fully valid transaction):

    01000000                      # nVersion
    00 03                         # marker + flag (SegWit + segOP)

    01                            # vin_count = 1
    aa..aa (32 bytes)             # prevout hash
    00000000                      # prevout index
    00                            # scriptSig_len
    ffffffff                      # nSequence

    02                            # vout_count = 2

    # vout0: P2SOP commitment (example script only)
    0000000000000000              # value = 0
    23                            # scriptPubKey_len = 35
    6a 23 534f50 <32-byte commit> # OP_RETURN "SOP" <commit>

    # vout1: P2WPKH
    a086010000000000              # value
    16                            # scriptPubKey_len = 22
    00 14 00112233445566778899aabbccddeeff00112233

    # witness for the input (flag & 0x01)
    02                            # number of stack items
    47 <sig71>                    # signature
    21 <pub33>                    # pubkey

    # segOP section (flag & 0x02)
    53                            # segop_marker
    01                            # segop_flag (v1)
    16                            # segop_len = 22 bytes
    01 10 7365674f5020544c562074657374  # TLV type 1
    02 04 00000001                      # TLV type 2

    00000000                      # nLockTime

## 7. IDs and Hashing

segOP preserves existing transaction identifiers and optionally allows implementations to define an extended ID.

| ID        | Includes                                        | Purpose                                |
|----------|--------------------------------------------------|----------------------------------------|
| txid     | nVersion + vin + vout + nLockTime                | Legacy transaction ID (unchanged)      |
| wtxid    | As per BIP141 (incl. witness, excludes segOP)    | SegWit ID (unchanged)                  |
| fullxid* | Entire extended tx (incl. segOP)                 | Optional extended/debugging identifier |

\* `fullxid` is not a consensus concept; this specification does not standardise any particular RPC name or require nodes to expose it. Implementations MAY define a debugging / extended ID that hashes the full extended serialization.

### 7.1 Legacy txid

Unchanged. Computed exactly as today:

- Serialize transaction without `marker`, `flag`, witness data, or segOP section.
- Compute double SHA-256.

segOP and witness sections MUST NOT affect `txid`.

### 7.2 wtxid (Unchanged)

Follows existing BIP141 semantics; does not include segOP in the hash.

### 7.3 fullxid (Optional)

Defined (if implemented) as:

- Serialization **including** `marker`, `flag`, witness data, and segOP section.
- `fullxid = SHA256d(serialization_with_marker_flag_witness_segop)`

Consensus does not depend on `fullxid`.

## 8. Weight and Fee Accounting

segOP bytes are charged at full base weight:

    segop_weight = 4 * segop_bytes

Total block weight becomes:

    block_weight = base_weight + witness_weight + segop_weight

Where:

- `base_weight` and `witness_weight` are as defined in BIP141.  
- `segop_bytes` is the value of `segop_len` for each transaction.  

### 8.1 Per-Transaction segOP Limit (Consensus)

Consensus rule:

    segop_bytes <= MAX_SEGOP_TX_BYTES

For segOP v1:

- `MAX_SEGOP_TX_BYTES = 64_000` (64 KB)

Transactions that exceed `MAX_SEGOP_TX_BYTES` MUST be invalid.

### 8.2 Per-Block segOP Limit (Policy)

It is RECOMMENDED that nodes and miners apply a policy limit on total segOP usage per block:

    sum(segop_bytes for all segOP tx in block) <= MAX_SEGOP_BLOCK_BYTES

Suggested default:

- `MAX_SEGOP_BLOCK_BYTES = 400_000`

This is deliberately defined as **policy**, not consensus, unless a later soft-fork explicitly promotes it to a consensus rule.

## 9. P2SOP – Pay-to-SegOP Output Type
### 9.1 Purpose

P2SOP is the on-chain commitment and signal for segOP payloads:

- Indicates that a transaction carries segOP data.  
- Binds the payload by hash.  
- Provides a stable index for external systems to locate associated payloads.  

### 9.2 Script Template

P2SOP uses a dedicated OP_RETURN script:

```
OP_RETURN 0x25 5032534f50 <32-byte commitment>
# Hex: 6a 25 5032534f50 <32-byte commitment>
```

Readable as:

```
OP_RETURN "P2SOP" <32-byte hash>
```

Properties:
- Unspendable (standard OP_RETURN semantics).
- Easy to scan for the literal ASCII string “P2SOP”.
- Serves as the canonical segOP commitment output.

### 9.3 Commitment Definition

segOP commitment uses a tagged-hash construction for domain separation, similar in spirit to BIP340-style tagged hashes.

Let:

```
TAG = SHA256("segop:commitment")
```

Define:

```
TAGGED_HASH(tag, m) = SHA256(TAG || TAG || m)   # where TAG = SHA256(tag)
```

Then:

```
segop_commitment = TAGGED_HASH("segop:commitment", segop_payload)
```

The 32-byte hash `segop_commitment` is placed as the final data push in the P2SOP script.

*Any change to this commitment scheme would require an explicit update before BIP finalisation or activation.*

### 9.4 Relationship and 1:1 Mapping Rules (Consensus)

For segOP v1, the mapping between segOP and P2SOP is strict:

- If `(flag & 0x02) == 0` (no segOP section):  
  - The transaction MUST NOT contain any P2SOP outputs.
 
- If `(flag & 0x02) != 0` (segOP present):  
  - The transaction MUST contain **exactly one** segOP section.  
  - The transaction MUST contain **exactly one** P2SOP output.

Any transaction that violates these constraints MUST be considered invalid under segOP consensus rules.

### 9.5 Example Pair

P2SOP output script (example):

```
6a23 534f50 3e7d08b77c3a5d60e8d01fcfc0e5aabde3f4b090c41211f1f8a9e7a71b76e9c5
```

segOP section (example):

```
    53 01 16
    01 10 7365674f5020544c562074657374  # TLV type 1 (text)
    02 04 00000001                      # TLV type 2 (app data)
```

The following MUST hold:

```
TAG = SHA256("segop:commitment")
segop_commitment = SHA256(TAG || TAG || segop_payload)
```

and the resulting 32-byte `segop_commitment` MUST equal the commitment embedded in the P2SOP output.

### 9.6 Node Validation Logic (Pseudo)

In pseudocode:

```
if (tx.flag & 0x02) { // segOP present
    auto p2sop_outputs = find_P2SOP_outputs(tx.vout);
    if (p2sop_outputs.size() != 1) {
        return TX_CONSENSUS_ERROR("invalid_p2sop_count");
    }

    if (!tx.has_segop_section || tx.segop_len == 0) {
        return TX_CONSENSUS_ERROR("missing_segop_section");
    }

    if (tx.segop_len > MAX_SEGOP_TX_BYTES) {
        return TX_CONSENSUS_ERROR("segop_too_large");
    }

    if (!is_valid_tlv_sequence(tx.segop_payload)) {
        return TX_CONSENSUS_ERROR("segop_tlv_invalid");
    }

    uint256 commit = p2sop_outputs[0].commitment;
    uint256 tag = SHA256("segop:commitment");
    uint256 expected = SHA256(tag || tag || tx.segop_payload);

    if (expected != commit) {
        return TX_CONSENSUS_ERROR("segop_commitment_mismatch");
    }
} else {
    // Non-segOP transaction MUST NOT use P2SOP
    if (find_P2SOP_outputs(tx.vout).size() != 0) {
        return TX_CONSENSUS_ERROR("p2sop_without_segop");
    }
}
```

Result: each segOP transaction has exactly one P2SOP and one segOP payload; no orphan segOP sections, and no multiple P2SOP spam.

## 10. Node Storage, IBD, and Pruning

segOP follows the same fundamental pruning model as Bitcoin Core: nodes must validate everything during IBD and reorgs, but may discard old block data (including segOP payloads) once it is no longer needed for validation.

### 10.1 IBD (Initial Block Download)

During IBD, a segOP-aware node:
- Downloads full blocks, including segOP payloads.

For each block, for each segOP transaction:
- Parses the segOP section.
- Verifies segop_len <= MAX_SEGOP_TX_BYTES.
- Verifies TLV structure.
- Computes segop_commitment and checks it against the P2SOP output.

Nodes MUST NOT prune segOP payloads from blocks that:
- Have not yet been validated, or
- Are still within the range needed for reorg safety.

### 10.2 Consensus vs Storage

Consensus cares about:
- Correctness of segOP payloads at the time the block is accepted.
- Correct commitments in P2SOP outputs.
- Block headers, Merkle roots, and the UTXO set.
- Consensus does not require nodes to retain raw segOP payload bytes forever.

Storage behaviour is a local policy choice:
- A node MAY keep all segOP payload data (archival behaviour).
- A node MAY discard segOP payloads from sufficiently old blocks (pruned behaviour), after validation and outside the reorg window.

### 10.3 Example Pruning Policy

Implementations may expose a configuration such as:

`-prunesegopheight=N`

Meaning: 
- For blocks at height h <= N, the node may delete stored segOP payload bytes from its local block storage.

The node MUST still retain:
- Block headers and Merkle roots.
- Transaction data needed for txid, wtxid, and P2SOP commitments.
- Enough recent blocks (including segOP payloads) to safely handle reorgs.

A segOP-aware node that prunes payloads in this way:
- Remains a fully validating node.
- May legitimately refuse RPC requests for historical segOP payloads beyond the pruning point (e.g. returning an error such as “segOP data pruned – commitment only”).

### 10.4 Node Roles (Informal)

**Standard full node (segOP-aware, pruned):**
- Validates all segOP payloads during IBD and as new blocks arrive.
- Prunes segOP payloads and block bodies after they are sufficiently buried.
- Mirrors Bitcoin’s existing pruned-node behaviour.

**Archival segOP node:**
- Retains all segOP payloads and full block data.
- Can serve historical segOP data to wallets, explorers, and protocols.

**Legacy node (non-segOP-aware):**
- Behaves exactly like current non-segOP Bitcoin Core.
- Ignores segOP sections; does not enforce segOP rules.
- Accepts any block valid under legacy rules.

## 11. Compatibility Summary

- `txid` is unchanged.
- Existing SegWit behaviour (`wtxid`, `weight calculation`) is unchanged.

segOP adds:
- A new, post-witness data section.
- An additional weight term.
- A P2SOP-based tagged commitment check.
- Blocks valid under segOP rules form a subset of blocks valid under legacy rules:
- segOP is a soft-fork compatible extension to Bitcoin’s transaction format.

## 12. Summary

- segOP introduces a post-witness, TLV-structured, prunable data lane for arbitrary and structured payloads.
- segOP data is fully fee-paying at 4 WU / byte.
- Each segOP transaction has exactly one P2SOP output and one segOP payload, bound by a tagged SHA256 commitment.
- Pruning behaviour mirrors existing Bitcoin node pruning: validate everything, optionally discard old payloads.
- Archival nodes can retain all segOP data; normal nodes do not have to.
- The design enables clean fee accounting, optional storage offloading, and future structured protocols (including quantum-safe schemes and rollups) without altering Bitcoin’s script semantics or transaction IDs.

---

### Appendix A – Constants (segOP v1)

| Constant              | Value     | Type      | Description                                  |
|-----------------------|-----------|-----------|----------------------------------------------|
| MAX_SEGOP_TX_BYTES    | 64,000    | Consensus | Max segOP payload per transaction            |
| MAX_SEGOP_BLOCK_BYTES | 400,000   | Policy    | Recommended max segOP bytes per block        |
| TAG_SEGOP_COMMIT      | "segop:commitment" | String    | Domain separation tag for P2SOP commitment   |
| SEGOP_VERSION         | 1         | Integer   | segOP v1 `segop_flag` value                  |


### Appendix B – Future Extensions (Reserved Flags)

| Bit / Hex | Name             | Description                                           |
|-----------|------------------|-------------------------------------------------------|
| 0x04      | segOP-Wit / Qsig | Reserved for future segregated sub-lane extensions   |
| 0x08–0x80 | Reserved         | Reserved for future extensions                       |


### Appendix C – Deployment (Suggested Outline)

| Parameter            | Description                                |
|----------------------|--------------------------------------------|
| Deployment mechanism | BIP8 (versionbits)                         |
| Bit                  | TBD                                        |
| Start time           | TBD                                        |
| Timeout              | TBD                                        |
| Activation threshold | e.g. 90% signalling over 2016 blocks       |


End of Specification
