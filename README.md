# segOP — Segregated OP_RETURN

**A proposed Bitcoin soft-fork extension for structured, fair, and prunable on-chain data.**  
**Author:** Defenwycke  
**Status:** Experimental  
**License:** MIT

## Overview

**segOP (Segregated OP_RETURN)** introduces a **dedicated data lane** to Bitcoin transactions — positioned after SegWit witness data and before `nLockTime`.  
It provides a **clean, structured**, and **full-fee** mechanism for carrying optional payloads without disturbing legacy consensus rules.

Think of segOP as to *arbitrary data* what **SegWit** was to *signatures* —  
a forward-compatible path that restores **fee fairness**, **data discipline**, and **node autonomy** while maintaining old-node harmony.

## Motivation

Bitcoin’s existing data channels (`OP_RETURN`, witness fields) have led to:

- **Unstructured payloads** that bloat the chain  
- **Discounted data lanes** that distort fee markets  
- **No pruning model**, burdening long-term node operators  

**segOP** proposes a minimal, structured remedy:

- All payload bytes charged at **4 WU / byte** (full fee weight)  
- Mandatory **TLV (Type-Length-Value)** encoding for structured parsing  
- Optional **payload pruning** after sufficient block depth  
- Full **soft-fork and legacy compatibility**

## Purpose and Philosophy

segOP does **not** exist to expand data usage — it exists to **contain and clarify it**.  
By isolating arbitrary data into a dedicated, fee-fair lane, segOP enables nodes to **quarantine**, **prune**, or **archive** data on their own terms.

At the same time, the structured TLV format opens new technical frontiers:
- **Quantum-secure signature anchors (Qsig)**  
- **Vault and covenant metadata**  
- **Rollup, oracle, and proof commitments**  
- **Post-quantum and hybrid layer experimentation**

These are *optional extensions*, not requirements — made possible because segOP provides a predictable, auditable home for advanced payloads.

## Status

| Field | State |
|-------|-------|
| Development | Active — see `/development/` and experiment logs |
| Consensus | Experimental (soft-fork proposal in progress) |
| Compatibility | Fully backward compatible with legacy nodes |
| Purpose | Research / Protocol design |
| Maintainer | Defenwycke |

## Resources

- 📘 [Draft Specification](.peer-review/docs/segop-spec.md)  
- 🧪 [Experiment Logs](./development/docs/dev-log.md/)  
- 💬 Discussion: Bitcoin-Dev mailing list (https://groups.google.com/g/bitcoindev/c/uhnM_EC0AQA)

## TL;DR

segOP isolates, structures, and fairly prices arbitrary data within Bitcoin —  
**not to encourage it, but to contain it — while unlocking future-proof use cases like quantum-secure signatures, vaults, and rollups.**


