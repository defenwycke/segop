# segOP Experiment Log #8

**Date:** 8 November 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)  

## Objective

Port the external Bash workflow into native Core RPCs — segopsend and decodesegoptx — and validate the end-to-end flow through both RPC interface and a minimal web wallet front-end.

## Summary of Experiment

### 1. Implementation Overview

segopsend RPC: replicates full pipeline internally.

Builds P2SOP commitment (534f50 || SHA256(payload)),

Creates, funds, and signs transaction,

Attaches segOP payload,

Broadcasts and optionally mines block.

decodesegoptx RPC: extracts segOP TLV structure from TXID or raw hex for inspection.

Both implemented under rpc/segop.cpp with help text and input validation.

### 2. Testing Procedure

Recompiled Core v30 with new RPCs enabled.

Started node and verified RPC availability via help segopsend and help decodesegoptx.

Executed:

```
bitcoin-cli -regtest segopsend segoptest bcrt1qg3asjp6yvw2hwxe4e76exd52e7wlhv4r2tu5y0 0.1 01020304 mine
```

Command output:

```
{
  "txid": "c3e09b7b3d6ef5...",
  "segop": {
    "version": 1,
    "size": 4,
    "hex": "01020304"
  },
  "blockhash": "1b2d...2ae7"
}
```

Queried the same TX through decodesegoptx:

```
bitcoin-cli -regtest decodesegoptx c3e09b7b3d6ef5...
```

Returned identical TLV structure, confirming decode accuracy.

### 3. Web Wallet Integration

Deployed lightweight web UI (React + RPC bridge) on localhost:8442.

UI functions: enter payload → submit segopsend → display decoded segOP.

Confirmed successful transaction broadcast and mined confirmation visible through both UI and CLI.

### 4. Verification

segopsend and segop_send.sh produce byte-identical serialized transactions.

No runtime or validation errors encountered.

Payload integrity, commitment hash, and block inclusion verified.

Legacy node remained unaffected (ignored segOP field).

## Discussion

Native RPC integration eliminates the dependency on external scripting and streamlines segOP transaction creation for wallets and external applications.
decodesegoptx provides a lightweight, developer-friendly inspection tool essential for debugging and UI integration.
Combined with the basic web wallet, this experiment demonstrates full-stack usability of segOP within Core’s RPC ecosystem.

## Notes

- Build base: feature/segop-v2 branch.
- Node runtime: regtest only.
- Wallet: segoptest.
- Payload vector: 01020304.

## Conclusion

Experiment #8 completes the transition from shell automation to native Core RPC control and introduces a working front-end demonstration.
segOP is now fully operable from payload composition to block inclusion through both command-line and web interfaces.

— Defenwycke, 2025
