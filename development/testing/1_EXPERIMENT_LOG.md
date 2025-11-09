# segOP Experiment Log #1
**Date:** 31 October 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)

## Objective

The purpose of this first experiment was to **prove that “segOP” (Segregated OP_RETURN) can be introduced without affecting legacy Bitcoin nodes**, while establishing a technical path for structured, full-fee data within Bitcoin transactions.

This is the **first practical implementation milestone** in evaluating segOP as a **soft-fork-safe, forward-compatible data extension**.

## Concept Overview

**segOP** is envisioned as to arbitrary data what **SegWit** was to signatures:  
a cleanly separated, optional, and verifiable extension area that restores **fee fairness** and **node choice**.

### Key design principles:

| Principle | Description |
|------------|-------------|
| **Backward-Compatible** | Old nodes remain fully functional — they ignore segOP data safely. |
| **Full-Fee** | No witness discount; all segOP bytes pay 4 WU/byte. |
| **Structured** | Payloads encoded via TLV (Type-Length-Value) for deterministic parsing. |
| **Optional Participation** | Nodes may prune or ignore segOP entirely with no consensus impact. |
| **Economically Neutral** | Normal transactions and fees unaffected. |

## Experimental Goals

1. **Demonstrate backward compatibility** — show that unmodified Bitcoin Core v30 nodes accept blocks produced by segOP-aware nodes.  
2. **Implement a minimal segOP RPC** to prove the code-path integration is possible without touching consensus logic.  
3. **Validate build integrity** — compile and run a modified Bitcoin Core (`bitcoin-segop`) with new RPCs registered cleanly.  
4. **Confirm runtime stability** — ensure daemon startup, mining, and block validation remain unchanged.  
5. **Prepare future extension points** for TLV parsing, pruning, and oracle-layer monetization.

## Implementation Summary (Phase 1)

| Component | File | Purpose |
|------------|------|----------|
| `rpc/rpc_segop.cpp` | New | Defines the RPC command `getsegopdata` returning placeholder data. |
| `rpc/register.cpp` | Modified | Registers `RegisterSegopRPCCommands(table)` during startup. |
| `init/bitcoind.cpp` | Modified | Hooks segOP RPC registration into the init sequence. |
| `segop_stub.cpp / .h` | New | Placeholder for future storage and retrieval logic. |
| `CMakeLists.txt` | Modified | Adds segOP source files to `bitcoind` target. |

**RPC Definition (stub)**  
```cpp
static RPCHelpMan getsegopdata()
{
    return RPCHelpMan{
        "getsegopdata",
        "Return segOP payload (placeholder, for testing only)",
        {
            {"blockhash", RPCArg::Type::STR_HEX, RPCArg::Optional::NO, "Block hash to query."},
        },
        RPCResult{
            RPCResult::Type::STR, "payload", "segOP payload string (currently empty)"
        },
        [&](const RPCHelpMan&, const JSONRPCRequest& request) -> UniValue {
            UniValue ret(UniValue::VSTR);
            ret.setStr("");
            return ret;
        }
    };
}
```
## Build & Test Results

| Action | Result |
|---------|---------|
| **Compile (`cmake --build . --target bitcoind`)** | Successful 100% build — no link errors |
| **Start daemon** | `bitcoind -regtest` runs cleanly |
| **Verify RPC appears** | `bitcoin-cli help` shows `getsegopdata "blockhash"` |
| **Execute RPC** | Returns empty string — no crash, valid JSON |
| **Compare old node** | Legacy `bitcoin-old` node mined + synced with segOP node on regtest with no conflicts |

## Observations

No consensus or validation code touched.
Old nodes remain unaware yet fully compatible.
RPC layer confirms clean modular injection point for segOP functionality.
Demonstrates feasibility of additive protocol extensions in the RPC/interface layer.

## Phase 1 Conclusion

The segOP RPC integration successfully proves that Bitcoin Core can be extended with new functionality
without disturbing legacy consensus or peer behaviour.
This establishes the baseline for further segOP research — namely, implementing data extraction,
storage, and fee-fair payloads.

## Next Steps (Phase 2 Plan)

1. **2.1 – Attach RPC to block index**  
   Retrieve `CBlock` by hash via `ChainstateManager`.

2. **2.2 – Parse payloads**  
   Search for segOP marker or TLV structure in transactions.

3. **2.3 – Return real data**  
   Encode and return segOP payload as Base64 or JSON.

4. **2.4 – Introduce pruning hooks**  
   Integrate node policy flags `-prunesegop`, `-keepsegopcommitments`.

5. **2.5 – Extend economic model**  
   Calculate full-fee accounting for segOP bytes.

