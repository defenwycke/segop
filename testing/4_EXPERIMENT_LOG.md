# segOP Experiment Log #4

**Date:** 3 November 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)  

## Objective

Document the successful reconstruction of the segOP transaction layer inside Bitcoin Core v30, describe the method used to integrate segOP fields into both the mutable and immutable transaction classes, and record the first clean compile of a segOP-ready node daemon.

## Summary of Events

### 1. Rebuild of segOP Transaction Model

Reopened a clean v30 source tree under /root/bitcoin-segop/ and confined all custom code to /src/segop/.

Extended CMutableTransaction with a new member CSegOP m_segop, allowing TLV-encoded payloads to be attached without disturbing existing vin/vout structures.

Added a corresponding immutable field const CSegOP m_segop to CTransaction in src/primitives/transaction.h.

Patched both constructors in transaction.cpp to copy or move the segOP data from the mutable version into the immutable one.

Confirmed field ordering and const-safety to preserve deterministic hashing and witness computation.

### 2. Successful Build of segOP-Enabled Core

The previous header-dependency issues were eliminated thanks to the strict isolation of segOP headers (#include <segop/segop.h> only in transaction.h).

Compilation of bitcoind, bitcoin-cli, and unit tests completed without warnings or linker failures.

Verified full node startup on regtest with clean logs:

Bitcoin Core starting
Using data directory /root/.bitcoin-segop/regtest


RPC connectivity (getblockchaininfo) and block generation (generatetoaddress) worked as normal, confirming that segOP integration did not break legacy transaction parsing.

### 3. Internal Consistency Checks

Confirmed that the new immutable CTransaction object correctly retains segOP data through copy/move operations.

Verified that serialization hooks still function for witness data; segOP lane currently inert (no serialization yet) but structurally bound to each transaction.

ToString() output was extended to display segop: present when payloads exist.

### 4. Next Implementation Phase
| Stage | Task | Description |
|:--|:--|:--|
| 1 | **Serialization Hook** | Add segOP marker and flag bits to `SerializeTransaction()` after witness section, mirroring SegWit layout. |
| 2 | **Deserializer Update** | Extend `UnserializeTransaction()` to detect segOP flags and populate `m_segop`. |
| 3 | **RPC Interface** | Expose `getsegopdata` via a new `rpc/segop.cpp` module to query TLV payloads. |
| 4 | **Test Vectors** | Create sample transactions with embedded segOP TLVs and validate round-trip serialization. |
| 5 | **Genesis Re-Mine (for segOP flag)** | Optionally mine a new regtest genesis block with segOP marker to confirm compatibility. |

## Summary

Experiment #4 marks the first functional compile of Bitcoin Core with integrated segOP fields.
The transaction object model is now segOP-aware yet remains fully backward compatible.
Next step is to connect these structures to the actual serialization path, achieving full wire-level parity with SegWit while maintaining soft-fork safety.
