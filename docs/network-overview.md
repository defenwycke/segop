# Hyper Hash Network — System Overview  
### Version: v0.1 — November 2025  
**Author:** Defenwycke  

## 1. Purpose  

This document explains how the components of the **Hyper Hash Network** interact — from miners submitting shares to Treasury payouts and Oracle API revenue loops.  
It visualises both the **data plane** (block propagation, segOP, templates) and the **payment plane** (Lightning treasury flow).

## 2. Layered Architecture  

```
┌────────────────────────────────────────────────────┐
│                    MINERS                          │
│  (ASICs, Bitaxe, or GPU rigs running Stratum v2)   │
└────────────────────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────┐
│                HYPER NODES                         │
│  - Relay Stratum templates & blocks                │
│  - Local “Dark Mode” mining support                │
│  - Submit tips, heartbeats, and discovered blocks  │
│  - Forward to Hyper Core                           │
└────────────────────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────┐
│                  HYPER CORE                         │
│  - Node registry & heartbeat verification           │
│  - Mining job orchestration & block aggregation     │
│  - Report eligibility data to Treasury              │
└────────────────────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────┐
│                HYPER TREASURY                       │
│  - Lightning-based payout engine                    │
│  - Ledger publication (JSON + signatures)           │
│  - Governance & multisig control                    │
│  - Splits pool income: 0.5 % to Treasury / 0.5 % to Node Network |
└────────────────────────────────────────────────────┘
                         │
                         ▼
┌────────────────────────────────────────────────────┐
│                 ORACLE NODES                        │
│  - Full archival nodes (segOP-aware)                │
│  - Serve blockchain + TLV data via Lightning 402 API│
│  - Sync with other nodes over standard Bitcoin P2P  │
│  - Contribute 1 % of API revenue to Treasury        │
└────────────────────────────────────────────────────┘
```

## 3. Data Flow  

### 3.1 Block and Tip Propagation  

```
[ Miner ] 
   │ submits share → [ Hyper Node ] 
   │                   │
   │                   ├─ relay header/tip → [ Core ]
   │                   │
   │                   └─ local dark-mode (optional)
   │
   └─> if valid → [ Core ] → [ Bitcoin Node / RPC ]
```

- Miners connect to their nearest **Hyper Node** using Stratum v1 or v2.  
- Nodes broadcast updated tips and relay found blocks upstream to **Hyper Core**.  
- **Dark Mode** nodes still send tips and heartbeats but only accept work from their own miners.  
- Core aggregates tip latency across regions to maintain the fastest relay paths.  

### 3.2 segOP & Oracle Synchronisation  

```
[ Transaction with segOP payload ]
        │
        ▼
[ Bitcoin Network ] → [ Oracle Node Archive ]
        │
        ├─ TLV parse + Merkle verification
        ├─ Store in local archive
        └─ Serve via Lightning 402 API
```

- segOP data is TLV-encoded, priced at full weight, and stored by Oracle Nodes.  
- Nodes can prune segOP sections after a defined retention period; Oracles retain them indefinitely.  
- Oracles allow paid retrieval through the `/api/v1/segop/{txid}` endpoint.  
- The **paid API** drives recurring revenue and supports Treasury funding.

## 4. Payment Flow  

### 4.1 Pool Payouts  

```
[ Pool Block Reward ]
         │
         ├─ 0.5 % → Hyper Treasury
         ├─ 0.5 % → Node Network (Hyper + Oracle)
         └─ 99 % → Contributing Miners (share-weighted)
```

- Pool rewards are split automatically by the Treasury.  
- Node Network payments are distributed equally per eligible node.  
- Miners receive Lightning payouts proportional to submitted work.  
- The winning miner also receives the entire transaction-fee portion of the block.  

### 4.2 Solo Mining  

```
[ Solo Miner Block Reward ]
         │
         ├─ 0.5 % → Hyper Treasury
         ├─ 0.5 % → Node Network
         └─ 99 % → Solo Miner
```

Solo miners retain the full block subsidy and transaction fees (minus the 1 % fee).  
This keeps solo and pooled mining economically aligned while preserving independence.

### 4.3 Oracle Revenue Loop  

```
[ User API Call ]
   │
   ├─ Pay 1–10 sats via Lightning 402
   │
   ├─ Oracle Node processes request
   │
   ├─ 99 % → Oracle Operator
   └─ 1 % → Treasury (via /treasury/remit)
```

Oracle activity feeds the same economic circuit as mining — revenue → Treasury → Node Network.  
This ensures every satoshi entering the system strengthens decentralisation.

## 5. Protocol Interfaces  

| Interface | Direction | Description |
|------------|------------|-------------|
| **Stratum v2 / v1** | Miner → Node | Share submission and job updates. |
| **HTTP / gRPC** | Node ↔ Core | Registration, heartbeat, and block submission. |
| **Lightning 402** | Public → Oracle | Paid access to archive or segOP data. |
| **Lightning keysend** | Treasury → Nodes / Miners | Automatic payouts. |
| **Bitcoin P2P** | Core / Oracle ↔ Network | Standard Bitcoin block and TX propagation. |

## 6. Security and Privacy  

- **Signatures:** All node communications use Ed25519 or secp256k1 signatures.  
- **Transport:** HTTPS / TLS 1.3 minimum, with optional gRPC mTLS.  
- **Privacy:** Dark Mode isolates miners from public relay lists without reducing reward eligibility.  
- **Auditing:** All payouts and ledgers are signed by the Treasury and queryable via API.  
- **Open Participation:** Any node or Oracle can join — no central authority or approval required.  

## 7. Summary Diagram  

```
           ┌────────────────────────────┐
           │        Miners (Stratum)    │
           └─────────────┬──────────────┘
                         │
                         ▼
          ┌────────────────────────────┐
          │       Hyper Nodes          │
          │  - Relay / Dark Mode       │
          │  - Heartbeats / Tips       │
          └─────────────┬──────────────┘
                         │
                         ▼
          ┌────────────────────────────┐
          │        Hyper Core          │
          │  - Registry / Templates    │
          │  - Block Submission        │
          └─────────────┬──────────────┘
                         │
                         ▼
          ┌────────────────────────────┐
          │      Hyper Treasury        │
          │  - Lightning Payouts       │
          │  - Governance / Ledgers    │
          └─────────────┬──────────────┘
                         │
                         ▼
          ┌────────────────────────────┐
          │        Oracle Nodes        │
          │  - Archive + segOP API     │
          │  - 1% Revenue → Treasury   │
          └────────────────────────────┘
```

## 8. Key Design Traits  

- **Fully Decentralised:** Every node can run independently with equal rights to rewards.  
- **Fair Fees:** 1 % total network fee sustains the system transparently.  
- **Interoperable:** Compatible with Bitcoin Core, Stratum v2, and Lightning standards.  
- **Sustainable:** Oracles create recurring income through data services.  
- **Inclusive:** Dark-mode and public operators share equal footing.  

*© 2025 Hyper Hash Network — Authored by Defenwycke — CC BY-SA 4.0*
