# segOP-Extended Transaction Specification

**Name:** segOP (Segregated OP_RETURN)  
**Author:** Defenwycke  
**Version:** draft-1.1  
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

Every segOP byte is charged at **4 weight units / byte** (no SegWit discount).

**Commitment binding via P2SOP:**  

A dedicated OP_RETURN output commits to a tagged SHA256 of the segOP payload using the tagged-hash convention defined in §7.1:

```
segop_commitment = TAGGED_HASH("segop:commitment", segop_payload)
```

where TAGGED_HASH(tag, msg) is defined in §7.1 as:

```
TAG = SHA256(tag)
TAGGED_HASH(tag, msg) = SHA256(TAG || TAG || msg)
```

Unified marker + flag signaling:

```
marker = 0x00
```

flag bitfield:

`0x01` = SegWit present

`0x02` = segOP present

`0x04` and above = reserved

Prunable payloads:

Nodes must fully validate segOP payload bytes on block acceptance.

After validation and after the payload falls outside their retention window, nodes MAY prune only the segOP payload bytes, while retaining all consensus-critical block, tx, script, and commitment data.

Soft-fork compatible:

- `txid` and `wtxid` are unchanged; legacy nodes accept `segOP` transactions.

## 3. Transaction Wire Layout

Extended transactions appear on the wire as follows:

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
      [ segop_version    = 0x01 ]        # segOP v1
      [ segop_len     (varint) ]
      [ segop_payload (segop_len bytes) ]

    [ nLockTime (4, LE) ]
```

SegWit and segOP can appear independently or together.

### 3.1 Extended Transaction Serialization

segOP reuses the SegWit “marker + flag” mechanism defined in BIP144. No new top-level fields are introduced; instead, the vin count of 0x00 followed by a non-zero flag byte signals the presence of optional extensions.

There are therefore two encodings:

Legacy / non-extended:

```
nVersion
vin_count (≠ 0x00)
vin[0..]
vout[0..]
nLockTime
```

Extended (SegWit, segOP, or both):

```
nVersion
marker = 0x00        ; encoded as vin_count = 0x00
flag   ≠ 0x00        ; bitfield, see below
vin[0..]
vout[0..]
[witnesses...]       ; present if (flag & 0x01) != 0
[segOP lane...]      ; present if (flag & 0x02) != 0
nLockTime
```

### 3.2 Marker and Flag

The extended transaction format reuses the SegWit marker/flag mechanism:

- `marker` is always the single byte `0x00`, in the position where a legacy `vin_count` would appear.
- `flag` is the following 1-byte **global extension bitfield**.

In segOP v1 the global `flag` byte uses:

- bit 0 (`0x01`): SegWit present  
- bit 1 (`0x02`): segOP present  
- bits 2–7 (`0x04`–`0x80`): reserved for future extensions

The full normative definition and examples for `marker` and `flag` are given in §4.

### 3.3 segOP lane placement

When the **segOP bit in the global flag** is set (`flag & 0x02 != 0`), the segOP lane is encoded after all inputs and outputs, and after any SegWit witness data:

```
nVersion
marker = 0x00
flag
vin[0..]
vout[0..]
[witnesses...] ; if (flag & 0x01) != 0
segop_marker = 0x53 ; ASCII 'S'
segop_version = 0x01 ; segOP v1
segop_len (CompactSize) ; total payload bytes
segop_payload [0..len-1] ; TLV-encoded payload
nLockTime
```

Any value of `segop_marker` other than `0x53` MUST be rejected as invalid.

`segop_version` is a **version byte**, not a bitfield. For segOP v1 it MUST be `0x01`. Future versions MAY use different values, but all segOP sections MUST still follow this placement: after any SegWit witness data and before `nLockTime`.



### 3.4 segOP and Transaction Weight

For the purposes of transaction weight, segOP behaves like non-witness data:

- segOP bytes are counted in the stripped size of the transaction.
- segOP bytes are not counted as witness and receive no discount.
- segOP does not change how txid or wtxid are defined:
    - txid is computed over the legacy non-witness serialization that excludes marker, flag, witness, and segOP (see §7.2.1).
    - wtxid is computed as in BIP141 and also ignores segOP bytes (see §7.2.2).

For weight, implementations MUST:

```
stripped_size = size of the transaction when encoded without any witness data, but including segOP marker/version/length/payload if present.

witness_size  = size of all SegWit witness data (if any)

weight = (stripped_size * 4) + witness_size
```

Put simply:

- segOP bytes are part of the stripped/base transaction for weight.
- segOP bytes are charged at the full 4 WU / byte rate.
- segOP bytes do not appear in the txid or wtxid serializations.

## 4. Marker and Flag Definition

### 4.1 Marker and Flag

| Field  | Size | Value | Purpose |
|--------|------|--------|----------|
| marker | 1    | 0x00   | Signals extended transaction format |
| flag   | 1    | bitfield | Indicates presence of SegWit and/or segOP |

### 4.2 Flag Bits

| Bit | Hex  | Meaning |
|-----|------|----------|
| 0   | 0x01 | SegWit present |
| 1   | 0x02 | segOP present |
| 2   | 0x04 | Reserved |

### 4.3 Examples

| Flag | Binary     | Meaning           |
|------|------------|-------------------|
| 0x01 | 00000001   | SegWit only       |
| 0x02 | 00000010   | segOP only        |
| 0x03 | 00000011   | SegWit + segOP    |

For SegWit + segOP, bytes after `nVersion` are: `00 03`.

### 4.4 Parsing Rules

**Legacy Compatibility**  

Legacy nodes misinterpret `marker=0x00`, so upgraded nodes detect the marker–flag pair before normal parsing.

**Extended Detection**  

- If `flag & 0x01`, SegWit follows `vout`.
- If `flag & 0x02`, segOP follows SegWit (or directly after vout).

**Pruning Behaviour**  

After a block has been fully validated (including segOP payload and P2SOP commitment) and its segOP payload has been pruned, nodes MAY omit re-reading or re-validating the pruned segOP bytes during later history checks (for example, re-verifying old blocks), because any modification to the segOP payload would change the P2SOP commitment, Merkle root, and block hash.

## 5. segOP Section

### 5.1 segOP Section Definition

A segOP section is appended:

```
[ segop_marker = 0x53 ]
[ segop_version   = 0x01 ]
[ segop_len    <varint> ]
[ segop_payload ]
```

A segOP section is defined as the contiguous sequence:

```
[ segop_marker ][ segop_version ][ segop_len ][ segop_payload ]
```

A transaction with segOP present MUST contain exactly one such segOP section.

Consensus:

- `segop_version` MUST be `0x01` for v1.
- `segop_len` MUST NOT exceed `MAX_SEGOP_TX_BYTES`.
- Payload length MUST match encoding exactly.
- The transaction MUST contain exactly one `P2SOP output`.
- The transaction MUST contain exactly one `segOP` section.
- `P2SOP` MUST NOT appear in non-segOP transactions.
- `segOP` MUST appear only after `SegWit` (if present) and before `nLockTime`.

## 5.2 TLV Payload (Mandatory)

The segOP payload (segop_payload) is defined as a concatenation of one or more TLV records:

```
[type (1 byte)] [len (varint)] [value (len bytes)]
```

**TLV rules (consensus)**

- Each TLV record MUST be well-formed.
- `type` is a single byte (0–255).
- `len` is a Bitcoin-style varint, allowing lengths from 0 to `MAX_SEGOP_TX_BYTES`.
- `value` MUST be exactly `len` bytes.
- TLVs MUST appear back-to-back with no padding.
- Unknown TLV types MUST be skipped.
- The entire concatenated TLV stream MUST be exactly `segop_len` bytes.

Rationale

- A payload may consist of one large TLV (simple use cases).
- A payload may contain multiple TLVs (structured use cases).
- TLV structure allows:
  - Metadata + body separation
  - Multiple logical components (e.g., headers, proofs, commitments)
  - Backwards/forwards compatibility
  - Selective parsing
  - Explicit Encoding Guarantee

To avoid reviewer confusion:

In segOP v1, len is a Bitcoin varint. This allows TLVs to range from 0 bytes up to the full segOP payload limit. Applications may encode their entire payload in a single TLV or split it into multiple TLVs as needed.

## 6. Worked Example (SegWit + segOP, flag = 0x03)

The following is an illustrative (non-signed) transaction showing SegWit and
segOP together. Hex values are examples only.

```
(tx start)

  01000000                                # nVersion = 1

  00 03                                   # marker=0x00, flag=0x03 (SegWit + segOP)

  01                                      # vin_count = 1

    <32 bytes>                            # prevout_hash
    00000000                              # prevout_index
    00                                    # scriptSig length
    ffffffff                              # nSequence

  02                                      # vout_count = 2

    # vout0 — P2SOP commitment
    0000000000000000
    27
      6a                                  # OP_RETURN
      25                                  # PUSHDATA(37)
      50 32 53 4f 50                      # ASCII "P2SOP"
      <32-byte segop_commitment>

    # vout1 — P2WPKH
    a086010000000000
    16
      00
      14
      <20-byte pubkey hash>

  # SegWit witness
  02
    47 <signature>
    21 <pubkey>

  # segOP section
  53                                      # segop_marker 'S'
  01                                      # segop_version v1
  
  18                                      # segop_len = 24 bytes (varint 0x18)

    # TLV #1
    01                                    # type = 1
    10                                    # len  = 16 (varint 0x10)
      73 65 67 4f 50 20 54 4c 56 20 74 65 73 74 21 21
      # "segOP TLV test!!"

    # TLV #2
    02                                    # type = 2
    04                                    # len  = 4 (varint 0x04)
      00 00 00 01

  00000000                                # nLockTime

(tx end)
```

TLV size breakdown:

```
TLV1: 1(type) + 1(len varint) + 16(value) = 18 bytes
TLV2: 1(type) + 1(len varint) + 4(value)  = 6 bytes

Total = 24 bytes = segop_len
```

### 6.1 Example TLV Structures (Informative)

Below are example TLV layouts illustrating real-world uses.

#### 6.1.1 Example A — One Large TLV (simple apps)

A simple inscription, file, or large proof:

```
[type=0x10][len=varint(64000)][value...64,000-byte blob...]
```

Structure:

```
+--------+--------------------+--------------------+
| 0x10   | varint(64,000)     | 64,000 bytes data  |
+--------+--------------------+--------------------+
```

Use cases:

- Inscriptions
- Large proofs
- Merkleized application blobs
- ZK-STARK traces

#### 6.1.2 Example B — Metadata + Body

Payload includes metadata TLV + big content TLV:

```
01 0A                <10-byte metadata>
10 <varint(64000)>   <64,000-byte raw data>
```

Meaning:

```
TLV 0x01 → “metadata” (10 bytes)
TLV 0x10 → “main blob” (64 KB)
```

This allows light-weight parsers to read type 0x01 (metadata) and skip over the large 0x10 blob without parsing it.

#### 6.1.3 Example C — Multi-component L2 rollup

```
01 05                <“L2v1”>
10 20                <32-byte batch commitment>
11 20                <32-byte fraud-proof root>
12 A0                <160-byte validator set snapshot>
20 <varint(64000)>   <large state delta>
```

This provides a structured rollup batch:

```
Type  Meaning
0x01  Protocol version (“L2v1”)
0x10  Batch commitment root
0x11  Fraud/submission root
0x12  Validator set metadata
0x20  Actual state delta
```

#### 6.1.4 Example D — Vault metadata + auxiliary data

```
01 20   <vault policy blob>
02 04   <relative locktime>
03 01   <flags>
10 20   <backup key commitment>
```

This shows structured, multi-field metadata inside one segOP payload, where each TLV is small but semantically distinct.

#### 6.1.5 Example E — “Envelope + Body” pattern

A TLV envelope describing the content, then a raw TLV containing it:

```
01 0F   <“mime:application/json”>
02 50   <80-byte JSON>
```

Here:

- `0x01` declares the media type.
- `0x02` carries the actual JSON body.

#### 6.1.6 Example F — Multiple optional extensions

```
01 01                <version byte>
02 20                <public tag / ID>
03 20                <signature commitment>
10 <varint(60000)>   <~60 KB main blob>
11 20                <optional auxiliary root>
```

This illustrates:

- A small fixed “header” region (version / IDs / commitments).
- A large main blob (0x10).
- An optional extension TLV (0x11) that higher-layer protocols may or may not understand.

## 7. IDs and Hashing

### 7.1 Tagged-hash convention

segOP uses a standard tagged-hash construction, consistent with BIP340-style tagging. For any `tag` (ASCII string) and message `msg`:

```
TAG = SHA256(tag)
TAGGED_HASH(tag, msg) = SHA256(TAG || TAG || msg)
```

All tagged hashes in this specification (e.g. P2SOP commitments and fullxid) use this convention.

### 7.2 ID definitions

segOP preserves existing Bitcoin identifiers and introduces an optional extended identifier:

| ID | Includes | Purpose |
|----|----------|---------|
| txid | nVersion + vin + vout + nLockTime | Legacy transaction ID |
| wtxid | BIP141 witness-inclusive serialization | SegWit transaction ID |
| fullxid | Entire extended transaction serialization (see below) | Optional extended ID |

### 7.2.1 txid (unchanged)

`txid` is computed exactly as in pre-segOP Bitcoin:

Serialize the transaction without:

- Marker.
- Flag.
- Any SegWit witness data.
- Any segOP section.

Compute SHA256d (double-SHA256) over that legacy serialization.

segOP and witness fields MUST NOT affect txid.

### 7.2.2 wtxid (unchanged)

`wtxid` is unchanged from BIP141. The serialization used to compute `wtxid` remains exactly the BIP141 witness serialization and does **not** include the segOP section.

segOP extends the wire-format transaction by appending an additional segOP section after witness and before `nLockTime`, but this extended region is not part of the `wtxid` computation. 

`wtxid` remains the SegWit witness-inclusive transaction ID as defined in BIP141.

### 7.2.3 fullxid (segOP extended ID)

Implementations MAY compute an optional extended transaction identifier `fullxid` that commits to the entire segOP-extended serialization, including segOP bytes, using the tagged-hash convention in §7.1.

Let `extended_serialization` be the byte sequence:

```
nVersion ||
marker || flag ||
vin (all inputs, as in extended tx format) ||
vout (all outputs) ||
[witness data, if present] ||
[segOP section, if present] ||
nLockTime
```

Then:

```
fullxid_tag = "segop:fullxid" # ASCII string
fullxid = TAGGED_HASH(fullxid_tag, extended_serialization)
```

#### Properties

- `fullxid` **commits to segOP payload bytes**. Any modification to the payload, its TLV structure, or its declared length changes the `fullxid`, even though the legacy `txid` and `wtxid` remain unchanged.
- `fullxid` is **not a consensus identifier** in segOP v1. It is not used in block validation, script execution, or relay policy.
- `fullxid` is **stable across pruning**: it is defined over the logical extended serialization, regardless of whether a node has pruned segOP bytes for a given block. A pruned node MAY need to refetch segOP payload bytes in order to recompute `fullxid`.
- `fullxid` has no ordering or malleability constraints and MUST NOT influence consensus-critical behaviour.

#### Intended Uses (informative)

`fullxid` is designed for:

- explorers and transaction indexers,
- debugging and implementation tracing,
- higher-layer protocols that wish to reference the full segOP-extended form,
- archival and reconstruction tools (particularly when segOP payloads are pruned but still retrievable from peers).

`fullxid` MUST NOT be required by consensus, and nodes MUST remain valid segOP implementations even if they do not compute or store `fullxid`.

## 8. Weight and Fees

For block weight, segop_weight is added to the existing block weight as defined in BIP141; segOP bytes are charged at 4 WU/byte with no discount.

```
segop_weight = 4 * segop_bytes
```

Consensus max per-tx:

```
MAX_SEGOP_TX_BYTES = 64,000
```

Recommended policy per-block:

```
MAX_SEGOP_BLOCK_BYTES = 400,000
```

## 9. P2SOP – Pay-to-SegOP

### 9.1 P2SOP Commitment

Each segOP payload is bound to its transaction by a single P2SOP commitment output, which is an OP_RETURN script carrying a tagged hash of the segOP payload.

The canonical `scriptPubKey` for a P2SOP output in segOP v1 is:

```
27 6a 25 5032534f50 <32-byte segop_commitment>
```

Where:

```
27 — scriptPubKey length in bytes (39 decimal)

6a — OP_RETURN

25 — PUSHDATA(37) — push 37 bytes of data

50 32 53 4f 50 — ASCII "P2SOP" (5 bytes: P 2 S O P)

<32-byte segop_commitment> — 32-byte commitment to the segOP payload

Total pushed data = 5 + 32 = 37 bytes, so the full scriptPubKey length is 0x27 bytes (1 byte OP_RETURN + 1 byte push opcode + 37 bytes data).
```

The commitment segop_commitment is computed using the tagged-hash convention from §7.1:

```
segop_commitment = TAGGED_HASH("segop:commitment", segop_payload)
```

Example:

```
TAG = SHA256("segop:commitment")              # over the ASCII string
segop_commitment = SHA256(TAG || TAG || segop_payload)
```

This segop_commitment is what appears as the <32-byte segop_commitment> in the P2SOP scriptPubKey above.

### 9.2 Relationship and 1:1 Mapping Rules (Consensus)

For segOP v1, the relationship between segOP and P2SOP is strict:

- If `(flag & 0x02) == 0` (no segOP):  
  - The transaction MUST NOT contain any P2SOP outputs.

- If `(flag & 0x02) != 0` (segOP present):  
  - The transaction MUST contain **exactly one** segOP section.  
  - The transaction MUST contain **exactly one** P2SOP output.

Any violation MUST make the transaction invalid under segOP consensus.

### 9.3 Example Pair

P2SOP output script (example, matching §6 and §9.1):

```
6a25 5032534f50 <32-byte commitment>
```

Where:

- `6a` — `OP_RETURN`
- `25` — `PUSHDATA(37)` — push 37 bytes of data
- `50 32 53 4f 50` — ASCII `"P2SOP"` (5 bytes)
- `<32-byte commitment>` — `segop_commitment` (32 bytes)

Total pushed data = `5 + 32 = 37` bytes, and the full `scriptPubKey` length is 39 bytes (`0x27`), (1 byte `OP_RETURN` + 1 byte push opcode + 37 bytes data).

Corresponding segOP section (example):

```
53 # segop_marker = 0x53 ('S')
01 # segop_version = 0x01 (segOP v1)
18 # segop_len = 24 bytes

01 10 # type = 1, len = 16
73 65 67 4f 50 20 54 4c 56 20 74 65 73 74 21 21
# "segOP TLV test!!" (16 bytes)

02 04 # type = 2, len = 4
00 00 00 01 # application-specific value (4 bytes)
```

TLV sizes:

```
TLV1: 1(type) + 1(len varint) + 16(value) = 18 bytes
TLV2: 1(type) + 1(len varint) + 4(value)  = 6 bytes

Total segop_payload = 18 + 6 = 24 bytes = 0x18
```

For this example, the 32-byte `segop_commitment` placed in the P2SOP output is:

```
segop_commitment = TAGGED_HASH("segop:commitment", segop_payload)
```

### 9.4 Node Validation Logic (Pseudo)

```
if (tx.flag & 0x02) {
    auto outs = find_P2SOP_outputs(tx);
    if (outs.size() != 1)
        return error;

    if (!tx.has_segop_section)
        return error;

    if (tx.segop_len > MAX_SEGOP_TX_BYTES)
        return error;

    if (!is_valid_tlv(tx.segop_payload))
        return error;

    uint256 commit = outs[0].commitment;
    uint256 expected = TAGGED_HASH("segop:commitment", tx.segop_payload);

    if (expected != commit)
        return error;
} else {
    if (has_P2SOP_output(tx))
        return error;
}
```

# 10. Node Validation, Retention, and Pruning

segOP follows Bitcoin’s “validate everything, optionally prune old data” model but makes retention windows explicit.

## 10.1 Mandatory Validation at Tip

For every segOP-bearing block, a segOP-aware node MUST:

1. Obtain the **full segOP payload bytes**, either:  
   - from local storage (if within retention window), or  
   - by requesting them via `getsegopdata` (§11.4.1) from peers
     advertising `NODE_SOP_RECENT` or `NODE_SOP_ARCHIVE`.
2. Recompute the segOP commitment.
3. Verify the P2SOP commitment matches.
4. Only then mark the block valid and relay it.

Nodes MUST NOT relay or accept a segOP-bearing block as valid until all segOP payloads are validated.

A segOP-aware fully validating node MUST apply segOP validation rules to every segOP-bearing block it accepts to its active chain, regardless of its later pruning policy.

## 10.2 Validation Window (Mandatory Minimum)

Nodes MUST retain segOP payloads for:

```
W = 24 blocks
```

This **Validation Window**:

- Supports 1–4 block reorgs  
- Supports miner time-rolling  
- Prevents fetch/prune thrashing  
- Ensures reliable block relay

Applies regardless of operator settings.

## 10.3 Operator Window (Optional)

Nodes MAY retain segOP payloads further using:

```
-sopwindow=R     # R ≥ 0 blocks
```

- `R = 0` → only Validation Window  
- `R = 144` → ~1 day  
- `R = 288` → ~2 days  

This does not affect consensus.

## 10.4 Effective Retention Window

The Effective Retention Window `E` is not a separate window; it is simply the result of combining the mandatory Validation Window and the user-configured Operator Window into a single numeric retention horizon.

Effective window:

```
E = max(W, R)
```

Nodes MUST retain segOP payloads for:

```
heights ∈ [ tip - E + 1 … tip ]
```

For blocks:

```
height < tip - E + 1
```

segOP payload MAY be pruned.

Pruning MUST NOT remove:

- tx fields  
- P2SOP outputs  
- commitments  
- essential serialization

## 10.5 Node Profiles (Informative)

Below are the three types of node profiles.

### 10.5.1 Validation Window Node  
- `E = W = 24`
- Retains segOP payloads for exactly the most recent 24 blocks.
- Prunes segOP payloads older than 24 blocks.
- Fully validates new blocks using payloads from window E = 24.

### 10.5.2 Operator Window Node  
- `E > W`  
- Retains deeper history  
- Suitable for routing/services

### 10.5.3 Archive Window Node  
- Retains all segOP payloads for the entire chain history (no segOP pruning).  
- Serves historical segOP payload to other nodes, explorers, and L2 protocols.  
- Voluntary; not consensus-required, but practically useful for full-history IBD and external indexing.

### 10.5.4 Consensus vs Storage (Summary)

- Consensus requires that any segOP-aware fully validating node verify segop_len, TLV well-formedness, and P2SOP commitments for every segOP-bearing block it accepts to its active chain.
- Storage policy is independent: after validation, a node MAY prune segOP payload bytes for blocks older than its effective retention window `E` without affecting consensus.

## 10.6 IBD (Initial Block Download)

During IBD, a segOP-aware fully validating node:

- Downloads blocks from its peers (preferably from Archive Window peers when full history is required).
- For every segOP-bearing block that it **accepts to its active chain**, it MUST:
  - parse the segOP section,
  - verify `segop_len <= MAX_SEGOP_TX_BYTES`,
  - enforce TLV well-formedness (§5.2),
  - recompute and verify the P2SOP commitment (§9.1–9.4).

Implementations MAY apply the same kinds of IBD shortcuts used for script validation today (e.g. assumevalid-style optimisations), but these are considered local policy and are not part of segOP’s consensus rules.

# 11. segOP P2P Behaviour and Payload Relay

## 11.1 Payload

segOP payload bytes:

- Are relayed with mempool transactions  
- Are stored on disk within window `E`  
- Are not included in compact block messages  
- Are fetched via dedicated P2P messages as needed (§11.4)

## 11.2 Service Bits

segOP introduces two new **P2P service flags** that nodes use to advertise their segOP data-serving capabilities. These flags are part of the node’s `services` bitfield (e.g., in the `version` message and address gossip such as `addrv2`). They are **not** part of the transaction or block serialization, nor part of the segOP wire format.

These service bits communicate what a node *can serve*, not what a transaction *contains*.

### 11.2.1 NODE_SOP_RECENT

`NODE_SOP_RECENT` is set **only if** the node can serve segOP payload bytes for at least the most recent **Validation Window**:

```
W = 24 blocks
```

A node sets this bit if and only if:

```
min_retained_height ≤ tip_height − W + 1
```

It is typically true for:

- Validation Window nodes (`E = W = 24`),
- Operator Window nodes (`E > W`),
- Archive Window nodes (Full retention).

Here `min_retained_height` is the lowest block height on the node’s active chain for which the node still retains segOP payload bytes.

### 11.2.2 NODE_SOP_ARCHIVE

`NODE_SOP_ARCHIVE` is set only if the node retains segOP payload bytes for **all blocks from genesis to the current tip** (i.e. it does not prune segOP payload at all).

A node MUST clear this bit automatically if it prunes segOP payload bytes for any block on its active chain (i.e., if it no longer retains full history).

Archive Window nodes are not required by consensus but are practically necessary for:

- full-history segOP IBD,
- L2 protocols anchored into segOP,
- explorers, auditors, and long-range reconstruction tools.

## 11.3 Inventory Types

- `MSG_TX_SOP` — announces a segOP-bearing transaction  
- `MSG_SOPDATA` — identifies a segOP payload object  

Nodes MUST NOT advertise payloads they do not hold.

## 11.4 segOP Payload Messages

### 11.4.1 `getsegopdata`

```
getsegopdata {
    txids: [32-byte txid, ...]
}
```

Rules:

- Non-empty list  
- Max 64 txids  
- Only send to peers advertising `NODE_SOP_RECENT` / `NODE_SOP_ARCHIVE`

### 11.4.2 `segopdata`

```
segopdata [
    {
        txid:       32 bytes
        sopver:     1 byte
        soplen:     varint
        soppayload: soplen bytes
    },
    ...
]
```

Nodes MUST return exactly `soplen` bytes for each entry.

If payload is pruned: respond with `notfound`.

## 11.5 Segmented Responses (Optional)

```
{
    txid: 32 bytes
    sopver: 1
    soplen: varint
    offset: varint
    chunk_data: bytes
    is_last_chunk: bool
}
```

Receivers MUST reassemble before commitment validation.

## 11.6 Block Relay Semantics

segOP follows the same relay model as SegWit: full blocks contain all data, while optimised block transports omit heavy data and require on-demand fetching.

### 11.6.1 Full Block Relay (`block` message)

Full blocks sent over P2P via the `block` message MUST include the full segOP section, including:

- `segop_marker`
- `segop_version`
- `segop_len`
- `segop_payload` (the actual bytes)

A node receiving a full block has all information required to:

- validate TLV structure
- recompute the P2SOP commitment
- validate commitments before accepting the block

Full blocks stored on disk (`blk*.dat`) SHOULD include segOP payload bytes for blocks within the node’s effective retention window `E`. Implementations MUST ensure that segOP payload bytes for heights in `[tip − E + 1 … tip]` are stored durably somewhere (whether inline in `blk*.dat` or in auxiliary files). Older blocks MAY have their segOP payload pruned.

### 11.6.2 Optimised Relay (cmpctblock and similar transports)

Compact blocks and other fast-relay formats (e.g., header-first, FIBRE-like schemes):

MUST NOT include segOP payload bytes.

MUST include:

- block header  
- transaction IDs / short IDs  
- witness flags (if applicable)  
- enough information to identify P2SOP outputs and their 32-byte commitments (for example, by including full scripts for segOP-bearing transactions or by encoding the commitments separately)

Nodes receiving an optimised block must:

- validate header + transaction skeleton,
- fetch missing witness data (if SegWit applies),
- fetch missing segOP payload bytes via `getsegopdata`,
- validate P2SOP commitments,

and only then accept and relay the block.

### 11.6.3 Relay Requirement

Nodes MUST NOT forward segOP-bearing blocks until the segOP payload has been:

- acquired (if missing), and
- validated against the P2SOP output.

Nodes MUST NOT relay partially validated blocks.

## 11.7 Compact Blocks

Compact blocks SHOULD include a segOP bitmap or equivalent metadata whenever segOP-bearing transactions are present, but segOP payload bytes MUST NOT appear inside compact block messages. This specification does not fix a specific compact block encoding.

## 11.8 IBD and Historical Reconstruction

During IBD:

- Prefer `NODE_SOP_ARCHIVE` peers for full history  
- Validate segOP for new blocks  
- Validation Window nodes do not need full-history re-fetch of pruned segOP payloads.

Archive nodes support:

- L2 rollups  
- Explorers/indexers  
- Forensic/audit reconstruction

segOP cleanly distinguishes **Validation Window**, **Operator Window**, and **Archive Window** modes.

## 11.9 DoS Mitigations, Rate Limits, and Policy Constraints

Implementations MUST enforce bandwidth, CPU, and message-frequency limits to prevent denial-of-service attacks relating to segOP payload requests.
All requirements in this section apply to the `getsegopdata` and `segopdata` messages defined in §11.4.

### 11.9.1 Per-Message Limits

Each `getsegopdata` message:

- MUST contain **at most 64 txids**.  
- MUST NOT exceed **4 KB** serialized.  
- MUST NOT contain duplicate txids.  
- SHOULD be ignored or penalized if malformed.

Each `segopdata` message:

- MUST contain at most **16 payload entries**.  
- MUST NOT exceed the policy-defined outbound limit (default **128 KB**, hard cap **256 KB**).  
- MUST NOT exceed **4×** the size of its corresponding inbound request.

Violations MUST result in the message being discarded and MAY contribute to misbehavior scoring.

### 11.9.2 Per-Peer Rate Limits

Nodes MUST enforce:

```
max_getsegopdata_per_minute = 64
```

Excessive requesting MUST cause the node to stop serving the peer and MAY count toward a ban score.

Peers requesting >256 KB/minute (policy) MAY be deprioritized or disconnected.

### 11.9.3 Global Backpressure and Bandwidth Caps

Nodes MUST enforce global bandwidth limits:

```
max_outbound_segop_bandwidth = 1 MB / 10 sec
max_inbound_segop_bandwidth  = 1 MB / 10 sec
```

Nodes MAY delay or queue responses under congestion.

### 11.9.4 Segmented Transfer Limits

For segmented transfer (§11.4):

- Abort if chunk timeout > **20 seconds**.  
- Abort if cumulative chunk size > declared `soplen`.  
- Penalize peers sending duplicate or inconsistent chunk offsets.  
- Discard partial buffers after reassembly or cancellation.

### 11.9.5 Invalid or Malicious Requests

Nodes MUST treat the following as misbehavior conditions and SHOULD feed them into their existing peer scoring / banning framework (e.g. `Misbehaving()` and `ban_threshold` in Bitcoin Core):

- Invalid `sopver`, incorrect `soplen`, or malformed TLV.
- Requests for txids a peer did not announce.
- Repeated requests for known-pruned payloads.
- Providing payload bytes that fail P2SOP commitment validation.

This specification does not define new scoring algorithms or ban thresholds; it only enumerates additional conditions that implementations SHOULD treat as misbehavior using their existing mechanisms.

### 11.9.6 Amplification Protection

To prevent response amplification:

- The outbound/inbound ratio MUST NOT exceed **4:1**.  
- Nodes MUST NOT serve segmented responses to peers exceeding bandwidth caps.  
- Nodes MUST NOT serve payloads outside their retention window unless they advertise `NODE_SOP_ARCHIVE`.

### 11.9.7 CPU and Memory Protections

Nodes MUST:

- Verify `soplen` before reading data.  
- Reject requests causing repeated TLV scans.  
- Limit concurrent chunk reassemblies (default policy: **8**).  
- Discard chunk buffers immediately after processing.

### 11.9.8 Interaction with Retention Windows

After initial IBD and validation, nodes SHOULD NOT fetch segOP payload deeper than their effective window `E` (§10.4) — that is, for blocks with:

```
height < tip_height − E + 1
```

— unless they are explicitly configured as Archive Window nodes and peers advertise `NODE_SOP_ARCHIVE`.

Requests for segOP payload outside a node’s retention horizon SHOULD be answered with `notfound`, unless the operator has explicitly opted into Archive behaviour.

### 11.9.9 Summary

Nodes MUST enforce:

- Per-message limits  
- Per-peer rate limits  
- Global bandwidth caps  
- Chunk-transfer protections  
- Misbehavior penalties  
- Amplification constraints  
- Memory/CPU protections

These mitigations ensure segOP cannot be used for bandwidth, CPU, or memory exhaustion attacks and remains consistent with Bitcoin's P2P security model.

# 12. Compatibility Summary

- `txid` is unchanged.  
- `wtxid` (BIP141) is unchanged.  
- SegWit semantics and script evaluation are unchanged.  
- segOP introduces:
  - a new post-witness data section,  
  - full-fee weight accounting for payload bytes,  
  - a P2SOP commitment output,  
  - mandatory TLV structure,  
  - prunable payloads with clearly defined retention windows.

Blocks valid under segOP rules form a **subset** of blocks valid under legacy rules. segOP is a **soft fork** extension.

# 13. Summary

- segOP introduces a post-witness, TLV-structured, fully fee-paying, prunable data lane.  
- Payload bytes are bound to the transaction via a P2SOP tagged-hash commitment.  
- Each segOP transaction contains exactly **one** P2SOP and **one** segOP section.  
- segOP defines explicit retention windows:
  - **Validation Window** (mandatory 24 blocks).  
  - **Operator Window** (user-configurable extension).  
  - **Archive Window** (retain all segOP payloads for the entire chain history, no pruning).  
- Payload relay uses dedicated P2P messages with strict DoS controls.  
- segOP preserves backward compatibility with legacy nodes and existing transaction IDs.  
- Provides a structured, future-proof foundation for data-bearing use cases including proofs, commitments, metadata, vault logic, and L2 anchoring.

Because every segOP payload is deterministically committed via a single P2SOP output, and that output is included in the transaction Merkle root and block hash, segOP-aware nodes can safely prune raw payload bytes once they fall outside their effective retention window `E` without weakening consensus. Any attempt to alter pruned segOP data would change the P2SOP commitment and therefore the block hash, which both legacy and segOP-aware nodes would reject. When deeper inspection or reconstruction is needed, nodes can request historical segOP payloads from peers advertising `NODE_SOP_ARCHIVE` using `getsegopdata` / `segopdata`, without changing consensus rules or requiring all nodes to store full-history payloads.

---

# Appendix A — Constants (segOP v1)

| Constant                  | Value       | Type       | Description |
|---------------------------|-------------|------------|-------------|
| MAX_SEGOP_TX_BYTES        | 64,000      | Consensus  | Maximum segOP payload per transaction. Enforced during mempool acceptance and block validation. |
| MAX_SEGOP_BLOCK_BYTES     | 400,000     | Policy     | Recommended per-block total segOP payload. Not consensus; guides mining and relay policy. |
| SEGOP_MARKER              | 0x53        | Constant   | ASCII `'S'`. Required byte indicating start of segOP section. |
| SEGOP_VERSION             | 1           | Integer    | Version byte for segOP v1 payloads (`segop_version`). |
| TAG_SEGOP_COMMIT          | "segop:commitment" | String | Tagged-hash domain for computing the P2SOP commitment. |
| TAG_FULLXID               | "segop:fullxid"     | String | Tagged-hash domain for computing the optional `fullxid`. |

## Notes

- **Consensus constants** must be enforced during transaction validation and block acceptance.
- **Policy constants** influence node behaviour but do not affect consensus.
- **Tagged-hash domains** (TAG_SEGOP_COMMIT, TAG_FULLXID) MUST be treated as exact ASCII strings.

---

# Appendix B — Future Extensions (Reserved Flags)

| Bit | Hex  | Name         | Description |
|-----|------|--------------|-------------|
| 2   | 0x04 | segOP-Wit / Qsig (reserved) | Placeholder for future segregated sub-lanes |
| 3–7 | 0x08–0x80 | Reserved | Future soft-fork extensions |

---

# Appendix C — Deployment (Suggested)

| Parameter            | Description |
|----------------------|-------------|
| Deployment mechanism | BIP8 (versionbits) |
| Bit                  | TBD |
| Start time           | TBD |
| Timeout              | TBD |
| Threshold            | e.g. 90% signalling over 2016 blocks |

---

# Appendix D - Diagrams (Informative)

TBD

---

# Appendix E - segOP Transaction Lifecycle (Informative)

TBD

---

End of segOP-Extended Transaction Specification

