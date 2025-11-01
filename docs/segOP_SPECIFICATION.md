# segOP-Extended Transaction Specification

**Author:** Defenwycke
**Version:** draft-1
**Date:** November 2025

---

## 1. Overview

This document defines an **extended Bitcoin transaction format** that unifies SegWit and segOP signalling under a single marker and bit-field flag.
It also introduces **P2SOP (Pay-to-SegOP)** — an output type anchoring structured payloads stored in a post-witness **segOP section**.

**Goals**

* Single unified marker for all extensions
* Backward-compatible with legacy parsers
* Full-fee accounting for segOP payloads (4 WU / byte)
* Deterministic placement: witness → segOP → locktime
* On-chain commitment via P2SOP output

---

## 2. Byte Order (wire layout)

Transactions appear on the wire in the following order:

(codehere)
[ nVersion (4) ]

[ marker (1) = 0x00 ]
[ flag (1) = bitfield {SegWit = 1, segOP = 2} ]

[ vin_count (varint) ]
 repeat vin_count times →
  [ prevout_hash (32, LE) ]
  [ prevout_index (4, LE) ]
  [ scriptSig_len (varint) ]
  [ scriptSig (bytes) ]
  [ nSequence (4, LE) ]

[ vout_count (varint) ]
 repeat vout_count times →
  [ value (8, LE) ]
  [ scriptPubKey_len (varint) ]
  [ scriptPubKey (bytes) ]

# if (flag & 0x01)

[ witness for each input ]
 for i in [ 0 .. vin_count-1 ]:
  [ wit_item_count (varint) ]
   repeat wit_item_count times →
    [ wit_item_len (varint) ]
    [ wit_item (bytes) ]

# if (flag & 0x02)

[ segop_marker (1) = 0x53 ] ; ‘S’
[ segop_flag (1) = 0x01 ] ; segOP version / feature bits
[ segop_len (varint) ]   ; payload length
[ segop_payload (segop_len bytes) ] ; TLV, Merkle root, etc.

[ nLockTime (4, LE) ]
(codehere)

---

## 3. Marker and Flag Definition

| Field  | Value     | Purpose                                |
| :----- | :-------- | :------------------------------------- |
| marker | 0x00      | signals “extended transaction”         |
| flag   | bit-field | defines which optional sections follow |

### Flag bits

| Bit | Hex  | Meaning           |
| :-: | :--- | :---------------- |
|  0  | 0x01 | SegWit present    |
|  1  | 0x02 | segOP present     |
|  2  | 0x04 | reserved / future |

**Examples**

| flag | Binary    | Meaning        |
| :--- | :-------- | :------------- |
| 0x01 | 0000 0001 | SegWit only    |
| 0x02 | 0000 0010 | segOP only     |
| 0x03 | 0000 0011 | SegWit + segOP |

Typical prefix for both: (00 03) after nVersion.

---

## 4. segOP Section

A segOP section is appended after witness data (if any) and before locktime.

(codehere)
[ segop_marker = 0x53 ] # ASCII ‘S’
[ segop_flag  = 0x01 ] # segOP version 1
[ segop_len   = <varint> ]
[ segop_payload (segop_len bytes) ]
(codehere)

### TLV structure inside payload

(codehere)
[type (1)] [len (1)] [value (len bytes)]
(codehere)

Example:

(codehere)
01 10 7365674f5020544c562074657374 # type 1, 16 bytes “segOP TLV test”
02 04 deadbeef # type 2, 4 bytes commitment
(codehere)

---

## 5. Worked Example (SegWit + segOP, flag = 0x03)

(codehere)
01000000                       # nVersion = 1
00 03                           # marker+flag (SegWit+segOP)
01                              # vin_count = 1
aa..aa (32 bytes)            # prevout hash
00000000                     # index
00                          # scriptSig_len
ffffffff                     # nSequence
02                             # vout_count = 2

# vout0 : P2SOP signal

0000000000000000             # value = 0
0b 6a 09 7365674f502d746167   # OP_RETURN “segOP-tag”

# vout1 : P2WPKH

a086010000000000 16 00 14 00112233445566778899aabbccddeeff00112233

# witness (flag & 0x01)

02 47 <sig71> 21 <pub33>

# segOP (flag & 0x02)

53 01 16 01 10 7365674f5020544c562074657374 02 04 deadbeef

00000000                      # nLockTime
(codehere)

---

## 6. Hashing and IDs

| ID              | Includes                                 | Purpose   |
| :-------------- | :--------------------------------------- | :-------- |
| txid (legacy)   | version + vin + vout + locktime          | unchanged |
| wtxid (SegWit)  | marker+flag + witness                    | existing  |
| fullxid (segOP) | marker+flag + witness + segOP + locktime | unique    |

---

## 7. Weight and Fee Policy

* segOP bytes → 4 WU / byte (full base weight)
* No witness discount
* Miners toggle via (-acceptsegop=1)
* Optional pruning (-prunesegopheight=N)

---

## 8. Backward Compatibility

* Legacy nodes ignore marker 0x00 and stop after vouts
* Same `txid` as before
* segOP-aware nodes parse payload if (flag & 0x02)
* Consensus unchanged → soft-fork-safe

---

## 9. P2SOP — Pay-to-SegOP Output Type

### Purpose

P2SOP is the **on-chain commitment and signal** linking a transaction’s vout to its segOP payload.

Functions → signal  • commit  • index.

---

### 9.1 Script Template

(codehere)
OP_RETURN 0x23 534f50 <32-byte commitment>
(codehere)

Readable as → (OP_RETURN "SOP" <32-byte hash>)

Hex pattern → (6a23 534f50 <hash>)

Unspendable but indexable.

---

### 9.2 Commitment Definition

(codehere)
segop_commitment = SHA256(segop_payload)
(codehere)

Alt (v1 extended):

(codehere)
segop_commitment = SHA256(SHA256(segop_payload) || segop_flag || segop_len)
(codehere)

---

### 9.3 Relationship Between Sections

(codehere)
┌────────────────────────────────────────────────────────────┐
│ nVersion                                                │
│ marker (00) + flag (03)                                 │
│ vin(s)                                                │
│ vout[0] = P2SOP (OP_RETURN "SOP" <commitment>)         │
│ vout[1] = normal spend                                │
│ witness (if flag&1)                                   │
│ segOP section (if flag&2)                             │
│ nLockTime                                            │
└────────────────────────────────────────────────────────────┘
(codehere)

Validation link → (SHA256(segop_payload) == commitment_in_P2SOP)

---

### 9.4 Example Pair

**P2SOP output**

(codehere)
6a23 534f50 3e7d08b77c3a5d60e8d01fcfc0e5aabde3f4b090c41211f1f8a9e7a71b76e9c5
(codehere)

**segOP section**

(codehere)
53 01 16 01 10 7365674f5020544c562074657374 02 04 deadbeef
(codehere)

Payload’s SHA-256 matches commitment.

---

### 9.5 Node Validation Logic (pseudo)

(codehere)
if (tx.flag & 0x02) {
 commit = extract_P2SOP_commitment(tx.vout)
 if (SHA256(tx.segop_payload) != commit)
  return TX_CONSENSUS_ERROR("segop_commitment_mismatch");
}
(codehere)

---

### 9.6 Pruning and Archival Policy

* Nodes may prune segOP payloads after N blocks
* Indexers keep (commitment, txid, height)
* Hyper Oracle archives retain (commitment, payload) for paid API

---

### 9.7 Summary Table

| Field / Section   | Purpose            | Notes                       |
| :---------------- | :----------------- | :-------------------------- |
| vout[P2SOP]       | Commitment signal  | OP_RETURN with “SOP” prefix |
| segOP section     | Payload storage    | TLV / Merkle / arbitrary    |
| segop_commitment  | SHA256(payload)    | Verification link           |
| flag bit 0x02     | Announces segOP    | Parser trigger              |
| segop_marker 0x53 | Section identifier | ASCII ‘S’                   |

---

## 10. Summary

* **Unified marker:** 0x00
* **Flag bits:** SegWit = 1, segOP = 2
* **Order:** [core tx] [witness?] [segOP?] [locktime]
* **Commitment:** P2SOP anchors payload
* **Weight:** 4 WU / byte for segOP data
* **Compatibility:** fully soft-fork-safe

segOP provides a **structured, full-fee, verifiable data lane** within Bitcoin transactions — restoring fee fairness and enabling archival data markets without breaking consensus.

---

*(End of spec)*
