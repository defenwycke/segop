# segOP Experiment Log #3  
**Date:** 2 November 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)

## Objective
Document the issues encountered while integrating segOP into Bitcoin Core v30, summarize lessons learned from the failed merge attempts, and outline the plan to rebuild a clean, traceable segOP layer separated from the original Core codebase.

## Summary of Events

### 1. Attempted segOP Integration into v30
- Began direct modification of the Bitcoin Core v30 source to embed segOP serialization hooks inside (transaction.h), (consensus/merkle.cpp), and (rpc/rawtransaction.cpp).  
- The goal was to maintain segOP logic natively while preserving upstream compatibility.  
- Compilation initially succeeded, but multiple inter-file include conflicts and circular dependency errors appeared when segOP structures were referenced outside of (src/segop/).

### 2. Key Issues Encountered
- **Header dependency chaos:** Adding segOP includes ((#include "segop/segop.h")) to core transaction files caused forward declaration conflicts and ambiguous symbol resolution.
  
- **Core file contamination:** Once segOP code was manually spliced into consensus and RPC layers, it became difficult to distinguish our additions from Core’s original logic. This made debugging and future merges nearly impossible.

- **Build regression:** Partial merges produced linker errors in (libbitcoin_common.a) and failed symbol resolution for (segop::DecodeTLV). These cascaded into RPC build failures during (make install).

### 3. Decision: Segregate segOP from Core
To restore clarity and maintain upstream compatibility, we decided to **rebuild segOP as a clean, modular layer** rather than patching Core directly.  
All segOP code will be isolated under (src/segop/), with **no direct edits** inside Bitcoin Core’s original directories.

Each addition to Core-facing files will be minimal and annotated using this convention:

```cpp
//segOP code start>>>
// Core modification here
//segOP code end<<<
and single-line inclusions tagged with (//segOP) at the end for traceability.
```

### 4. Planned Rebuild Process

- Start Fresh: Clone a clean v30 source tree and create a new feature branch (feature/segop_clean).
- Namespace Isolation: Implement all segOP classes, TLV logic, and serialization utilities within (src/segop/).
- Interface Hooks: Instead of editing core transaction structures, expose segOP via a lightweight interface (segop_interface.cpp) that Core can call optionally.
- RPC Refactor: Rebuild (rpc_segop.cpp) as an external RPC module registering its commands dynamically at startup, keeping the Core RPC table untouched.
- Code Annotation & Auditing: Every segOP-related line clearly marked for easy diffing, auditing, and upstream merge tracking.

### 5. Outcome
While yesterday’s build did not produce a working binary, it clarified the structural pitfalls of modifying Core directly.
The new approach will give us:

- A fully traceable codebase.
- Clean separation for future merges.
- Easier debugging, review, and documentation of every segOP addition.
