# segOP — Segregated OP_RETURN  
### A proposed Bitcoin soft-fork extension for fair, structured on-chain data  

## Overview

**segOP (Segregated OP_RETURN)** is an experimental Bitcoin protocol extension that introduces a **dedicated, structured, and full-fee data lane** within Bitcoin transactions — without breaking backward compatibility.  

Think of segOP as **to arbitrary data what SegWit was to signatures**:  
a clean, forward-compatible path that restores fee fairness and node choice while maintaining old-node harmony.  

## Motivation

Bitcoin today allows arbitrary data through `OP_RETURN` and witness fields, but these have led to:
- **Abuse of discounted witness space** (e.g. ordinals / inscriptions)
- **Unstructured payloads** that bloat the chain
- **No clear pruning or retention model**

segOP proposes a minimal, structured extension that:
- Enforces **full fee rates** for data (4 weight units per byte)
- Encapsulates payloads in **TLV (Type-Length-Value)** form
- Allows **optional pruning** and **archive-layer monetization**
- Remains **soft-fork safe and backward compatible**

---

Testing currently in progress: See progress in dev and experiment logs.
