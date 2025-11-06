# segOP Experiment Log #5

**Date:** 6 November 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)  

## Objective

To confirm end-to-end operation of the segOP transaction system under live regtest conditions:
serialization → validation → broadcast → block inclusion → on-chain retrieval.

## Summary of Events
#### 1. Full RPC and Consensus Integration

The new RPC command createsegoptx was finalized, allowing injection of arbitrary segOP payloads into signed transactions.
Consensus rules in (tx_check.cpp) were activated — each segOP transaction must include exactly one P2SOP output whose 32-byte commitment equals SHA256(segop_payload).

Any violation (missing / duplicate P2SOP or hash mismatch) correctly triggers:
```
bad-txns-segop-no-p2sop
bad-txns-segop-commitment-mismatch
```

All internal tests confirmed deterministic enforcement.

### 2. Construction of First Valid segOP Transaction

A clean wallet (segoptest) was funded on regtest.
Raw transaction assembled using (createrawtransaction) included a dedicated P2SOP output:

```
OP_RETURN 0x23 "SOP" <32-byte commitment>
```

Payload (01020304) was attached using:

```
createsegoptx "<signed_hex>" "01020304"
```

Resulting transaction (tail excerpt):

```
...5301040102030400000000
```

Decoder output confirmed the segOP section:

```
"segop": { "version": 1, "size": 4, "hex": "01020304" }
```

### 3. Broadcast and Block Inclusion

Initial submission without P2SOP rejected as expected (bad-txns-segop-no-p2sop).

After adding proper P2SOP, transaction accepted into mempool.

Mined into block (63698d822d4634e05c69d2d4068ca262e2c0c0e53076b27705fc33d95532bf31).

Retrieved via:

```
getrawtransaction df30f5d6...d4855 1 63698d82...
```

showing:

```
"in_active_chain": true,
"confirmations": 1,
"segop": { "version": 1, "size": 4, "hex": "01020304" }
```

### 4. Verification and Analysis

- Computed commitment inside P2SOP (9f64a7…e89b6a806a) matches SHA256(01020304).
- segOP payload serialized after witness data per specification.
- Legacy decoders remain compatible, ignoring the segOP section gracefully.
- Node RPCs (decoderawtransaction, getrawtransaction) now expose segOP fields directly.
- Consensus, policy, serialization, and RPC layers all function cohesively.

### Conclusion

Experiment #5 marks the first mined segOP transaction in Bitcoin Core v30 (regtest).
The full pipeline—from TLV payload creation to block-confirmed verification—has been proven stable.
This establishes segOP as a working, consensus-compatible extension ready for further testing, wallet integration, and eventual mainnet soft-fork discussion.

Status: segOP operational on-chain (regtest)
