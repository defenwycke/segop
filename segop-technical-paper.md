# segOP : Segregated OP_RETURN
# A Structured and Fee-Fair Data Extension for Bitcoin

## Technical Paper — October 2025
## Author : Defenwycke

## Abstract

Bitcoin’s four-million-weight-unit block limit was meant to price blockspace fairly, yet Segregated Witness introduced an asymmetry: witness bytes cost one weight unit while base bytes cost four.
That discount—intended for signatures—has become a subsidy for arbitrary data.
Large image and metadata “inscriptions” now fill much of each block while paying a fraction of what monetary transactions pay.
The result is a distorted fee market: blocks remain full but miners earn less and real users queue longer.

segOP (Segregated OP_RETURN) restores balance without a hard fork.
It defines a structured, priced data lane inside the transaction—full fee rate, capped at 100 KB, TLV-encoded and Merkle-verifiable.
Each payload pays the same 4 WU per byte as ordinary transaction data.
Economic modelling shows miner revenue roughly doubling and genuine payment throughput increasing about 75 %, while spam falls naturally.
The proposal is soft-fork safe, easily pruned, and backward compatible.

## 1 Introduction
### 1.1 Why This Matters

Bitcoin’s strength lies in simplicity: every byte competes equally for limited space.
Since 2017, witness discounting has broken that symmetry.
A 1 MB batch of signatures or data in the witness section now costs only ¼ of what it would in the base section.
That imbalance has turned the witness into a subsidised data store.
Ordinary users fund cheap on-chain storage for unrelated content, miners lose fees, and block propagation slows.

### 1.2 Goal of segOP

segOP re-introduces equal pricing.
It doesn’t forbid data; it merely isolates and meters it properly.
Like SegWit separated signatures for malleability reasons, segOP separates arbitrary data for economic fairness.
Every byte again pays its way.

## 2 Problem Statement
### 2.1 The Fee Distortion

Rule	Current Value	Impact
Block weight limit	4 000 000 WU	hard cap
Witness discount	¼ WU / B	data cheap
Max script element	520 B	protects validation
UTXO retention	permanent	growing state

Because witness data is so cheap, 100 KB of arbitrary content weighs 100 000 WU instead of 400 000 WU.
Miners still fill blocks to 4 M WU, but earn less; users compete against discounted junk.
The imbalance is economic, not cryptographic.

### 2.2 Consequences

Fee market signal blurred — cheap spam masks real demand.
Throughput loss — genuine tx count falls ≈ 40–60 %.
Revenue loss — full blocks yield low fees.
Propagation load — bigger blocks take longer to relay.
Without change, history repeats: larger chain, lower income, weaker incentives.

## 3 Proposal — segOP (Segregated OP_RETURN)

segOP introduces a new optional section after the witness.
It carries arbitrary structured data at full cost (4 WU/B) and is capped at 100 KB per transaction.
Presence is signalled by a new flag bit (0x02).
Legacy nodes ignore it; upgraded nodes parse and charge for it.

### 3.1 Design Summary

Parameter   	/ Rule                    / Purpose
---------------------------------------------------------------------
Encoding      / TLV (Type-Length-Value) / Self-describing, extensible
Maximum size  / 100 000 B	              / Stops bloat
Cost per byte / 4 WU	                  / Restores parity
Witness limit	/ 520 B	                  / Protects execution
Placement     /	After witness	          / Deterministic layout
Compatibility	/ Soft-fork safe	        / Old nodes ignore

### 3.2 Why Equal Pricing Matters

You can’t legislate away spam; you fix it by removing its subsidy.
segOP ends the discount, making large data pay the same rate as payments.
Abuse becomes uneconomic—no blacklists or heuristics needed.

### 3.3 Compatibility

Old nodes ignore bit 0x02.
txid excludes segOP data → no malleability risk.
Deployment via version-bits or miner policy.

### 3.4 Analogy

SegWit separated signatures for security; segOP separates data for fairness.
Both reduce friction between innovation and consensus.

## 4. Economic Model

segOP changes the economics of blockspace by removing the discount that allows data to underpay for its size.  
To understand its impact, we can model the block in three parts: normal transactions, discounted data (today’s reality), and segOP data (the proposed model).

### 4.1  Variables and Setup

Let:

- `Wb` = block weight limit → 4,000,000 weight units  
- `Wtx` = average transaction weight → 600 weight units  
- `S` = segOP payload size → 100,000 bytes  
- `WsegOP` = segOP weight = 4 × S = 400,000 weight units  
- `f` = fee rate in satoshis per virtual byte (sat/vB)  

We define the following relationships:

FsegOP = f × S
T = (Wb - 400,000 × n) / Wtx
Fblock = n × FsegOP + T × (f × 150)

Where:

- `n` is the number of segOP payloads in a block.  
- `FsegOP` is the fee paid by one segOP transaction.  
- `Fblock` is the total fee income for the block.

This gives a rough but useful estimate of how miner revenue scales when blocks contain different mixes of segOP data and normal transactions.

### 4.2  Scenario A — Current Situation (Discounted Witness Data)

In the current network, around sixty percent of block weight can be consumed by discounted witness data.  
That data pays only one quarter of the fee rate that normal bytes would, so the block fills up faster but generates less revenue.

A typical block might contain about 2,600 normal transactions and yield around 0.006 BTC in fees.

The network looks busy, but most of the block is effectively subsidised storage.

### 4.3  Scenario B — segOP Introduced (Full-Fee Data Lane)

With segOP active, arbitrary data still fits on-chain, but it now pays the same four weight units per byte as everything else.  
Assume three segOP payloads of 100 KB each per block.

Under these conditions, the block includes roughly 4,600 normal transactions and produces around 0.013 BTC in total fees.  
Miners earn more, users see more legitimate transactions confirmed, and blockspace pricing becomes honest again.

### 4.4  Scenario C — No Discounted Data

If the network were used purely for monetary transactions, a full block would contain roughly 6,600 standard transactions at an average total fee of about 0.010 BTC.  
This represents the upper bound for throughput but not necessarily the most balanced use of blockspace.

### 4.5 Interpretation

Spam deterrence: Data becomes too expensive to abuse; only purposeful data remains.
Fair market: Each byte competes equally, eliminating hidden discounts.
Higher throughput: More monetary transactions per block because junk is no longer cheap.
Miner incentive: Full blocks now pay full rewards; revenue rises naturally.
Stable propagation: Blocks remain smaller and more predictable in structure, improving relay speed.

segOP doesn’t require a new policy or central coordination—it simply restores the natural balance of the fee market through equal pricing.

## 5 Behavioural and Economic Implications

Spam deterrence — once data costs full rate, stuffing blocks is uneconomic.
Fair market — every byte competes equally.
Higher throughput — fewer bloated transactions.
Better miner income — full blocks now mean full fees.
Predictable propagation — smaller, uniform payloads reduce latency.

It’s a market fix, not a ban list.

## 6 Security and Consensus Considerations

segOP changes accounting, not consensus.
Validation logic stays deterministic; legacy nodes remain valid peers.

Risk                    / Mitigation
-----------------------------------------------------------------
Soft-fork compatibility	/ Unrecognised flag ignored by old nodes
Malleability            / segOP excluded from txid
Script safety	          / 520 B element limit unchanged
Resource load	          / ≤ 100 KB per tx caps memory
Replay risk	            / Flag bit isolated from version

### 6.1 Validation Flow (pseudocode)

```python

def validate_tx(tx):
    check_structure(tx)
    check_inputs_outputs(tx)

    if tx.flag & 0x01:  # SegWit
        validate_witness(tx)
        for w in tx.witness:
            for item in w:
                if len(item) > 520:
                    raise ScriptError("witness element too large")

    if tx.flag & 0x02:  # segOP
        seg = tx.segop
        if seg.length > 100_000:
            raise PolicyError("segOP payload too large")
        charge_weight(seg.length * 4)
        validate_tlv(seg.tlv)
```

The logic is simple: include data, pay for data.

## 7 Implementation and Flag Structure

Flag Bit / Meaning             / Present In	/ Cost Model
-----------------------------------------------------------------------
0x00     / Legacy (no witness) / Pre-SegWit / 4 WU/B
0x01     / SegWit only         / Post-2017  / 1 WU witness
0x02     / segOP only          / Future     / 4 WU segOP
0x03     / SegWit + segOP      / Mixed      / 1 WU witness + 4 WU segOP

Older software ignores 0x02, so segOP deploys as a soft fork with zero disruption.

## 8 Transaction Encoding Example (segOP Flag + P2SOP)

A working example showing how segOP coexists with SegWit, commented line-by-line.

```jsonc

{
  "version": 2,                        // transaction version
  "flag": 3,                           // 0x01 = SegWit, 0x02 = segOP → 0x03 = both

  "vin": [
    {
      "txid": "aa...aa",               // previous transaction hash
      "vout": 0,                       // output index
      "scriptSig": "",                 // empty for SegWit spend
      "sequence": 4294967295
    }
  ],

  "vout": [
    {
      "value": 0.01000000,             // spendable output
      "scriptPubKey": {
        "type": "witness_v0_keyhash",
        "hex": "0014abcdef..."
      }
    },
    {
      "value": 0.00000000,             // zero-value marker
      "scriptPubKey": {
        "type": "nulldata",
        "asm": "OP_RETURN OP_SEGOP",   // Pay-to-segOP marker
        "hex": "6a045345474f50"        // ASCII “SEGOP”
      }
    }
  ],

  "witness": [
    [ "3045...01", "02ab...cd" ]       // standard sig + pubkey
  ],

  "segop": {
    "length": 1024,                    // total segOP bytes (≤100 000)
    "tlv": [
      {
        "type": 1,                     // data hash
        "len": 32,
        "value": "d4d95998b5...e3"
      },
      {
        "type": 2,                     // declared data length
        "len": 2,
        "value": "03e8"                // 1000 bytes
      },
      {
        "type": 3,                     // optional Merkle root
        "len": 32,
        "value": "12ab...9f"
      }
    ]
  },

  "locktime": 0
}
```

Validation flow:
1. Detect flag & 0x02.
2. Reject if segop.length > 100 000.
3. Weight = segop.length × 4.
4. Witness elements ≤ 520 B.
5. Fee = (base_vbytes + segop.length) × feerate.

Field	  / Purpose	              / Effect
-------------------------------------------------------
flag:3  / SegWit + segOP active / full-price accounting
vout[1]	/ P2SOP output          / marker only
segop	  / structured payload    / TLV encoded
length  / total bytes           / hard cap enforced
tlv	    / typed entries	        / verifiable meta

## 9 Long-Term Storage and Pruning Strategy
### 9.1 Background

A node only needs the UTXO set and headers to enforce consensus; it doesn’t need to host everyone’s history forever.
Bitcoin Core already prunes full blocks to save space but cannot surgically drop arbitrary data inside them.
segOP fixes that: since its data are self-contained and fully paid, they can be forgotten safely once validated.

### 9.2 Pruning Process

Validate segOP payload and hash.
Record its type, length, hash commitment.
Delete payload after chosen horizon.
Keep headers and commitments for audit.
The node remains consensus-equivalent but disk-light.

### 9.3 Selective Retention

TLV Type / Example             / Keep? / Reason
-----------------------------------------------------
1        / Hash or Merkle root / yes   / audit proof
2        / Declared length     / yes   / accounting
10       / Raw payload         / no    / space saving

### 9.4 Policy Flags

-prunesegop=1
-prunesegopheight=2016
-prunewitness=1
-prunewitnessheight=2016
-keepsegopcommitments=1

### 9.5 Storage Projection

To understand the long-term effect of segOP on storage, it helps to model three realistic futures: the monetary-only baseline, the current discounted-data situation, and a segOP-priced and pruned network.

In a **monetary-only scenario**, blocks average around 1 megabyte of actual transaction data.  
At roughly 52,560 blocks per year, that equals about 52 gigabytes annually or roughly 525 gigabytes per decade.  
This represents the "clean" Bitcoin model — payments only, no arbitrary data.  
It’s the lower bound for full-node storage.

In the **discounted-data future** (the situation we already see today with Ordinals and similar use cases), most blocks reach 2.5 megabytes on average once the witness discount is exploited.  
That comes to around 1.3 terabytes per decade, and it keeps climbing if large cheap data continues to fill every block.  
In this world, node operators are forced to host other people’s content indefinitely while earning less from fees — unsustainable for the long term.

In the **segOP-priced and pruned model**, blocks still carry roughly 1 megabyte of payment data, but they may also include up to 1 megabyte of segOP payloads.  
The key difference is that segOP data is fully paid for and can be safely pruned after validation.  
A node that keeps a two-week rolling window of segOP data (about 2,000 blocks) would only need an additional 2 gigabytes of temporary disk space.  
After pruning, the total long-term requirement stabilises near the same 525-gigabyte range as the monetary-only case — effectively holding the line on growth.

In short, segOP allows Bitcoin to support data-rich use cases **without** forcing every node to store terabytes of history.  
Heavy users pay for their data, and those who want to keep it can, while everyone else can prune and move on.

### 9.6 Integrity After Pruning

Payloads may go, but commitments stay : block hashes still cover segOP roots, headers prove existence, archives can re-serve data verifiable by hash.

### 9.7 Data Retention Layers
┌─────────────────────────────────────────────┐
│                 BITCOIN NODE                │
└─────────────────────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────────┐
│      CORE CONSENSUS (UTXO + Headers)        │
└─────────────────────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────────┐
│  RECENT WINDOW (Blocks + Witness + segOP)   │
└─────────────────────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────────┐
│   PRUNED HISTORY (Old payloads deleted)     │
└─────────────────────────────────────────────┘
           │
           ▼
┌─────────────────────────────────────────────┐
│ ARCHIVE / ORACLE NODE (optional history)    │
└─────────────────────────────────────────────┘

### 9.8 Pruning Economics

10-year pruned node ≈ 500 GB.

10-year archive ≈ 1–3 TB.
Only voluntary archives bear that cost— not everyone.

### 9.9 Data Ownership and Access Model

Under current rules, full nodes host other people’s junk indefinitely.
segOP changes that by making data self-contained and prunable.

After validation, nodes may drop payloads while keeping hashes and lengths.
They stay in consensus without carrying unwanted content.

Those who want the data keep it; those who don’t prune it.

Archive (or oracle) nodes voluntarily retain all segOP payloads and offer them through APIs or Lightning-paywalled endpoints.
Because every payload carries its own Merkle root or SHA-256 hash, clients can verify authenticity independently.

No node is forced to store other people’s data; each operator chooses their own weight class.
It’s fee fairness at inclusion and storage fairness after confirmation — a complete economic loop.

## 10 Operational Profiles

### 10.1 Profile A — Home Node

Light validation on small hardware.

-prune=550
-prunesegop=1
-prunesegopheight=2016

Keeps two weeks of history; fully validating; tiny disk footprint.

### 10.2 Profile B — Mining / Economic Node

-prune=20000
-prunesegop=1
-prunesegopheight=8064
-keepsegopcommitments=1

Keeps 3 months of segOP for fee analysis and template testing.

### 10.3 Profile C — Archive / Oracle Node

-prune=0
-prunesegop=0
-keepsegopcommitments=1

Stores everything, serves API or Lightning requests, earns sats for data.

### 10.4 Profile D — Enterprise / Auditor

-prune=0
-prunesegop=1
-prunesegopheight=40320

Keeps 9 months of segOP for audit; drops old payloads.

### 10.5 Summary Table

Profile	/ Purpose	   / SegOP Policy        / Storage                 / Notes
--------------------------------------------------------------------------------------
A	      / Home       / Prune after 2 weeks / Approx. 500 GB (decade) / Light + private
B	      / Miner      / Keep 2–3 months     / Approx. 1 TB (decade)	 / Analytics
C	      / Archive    / Keep forever        / Approx. 1–3 TB (decade) / Monetisable
D	      / Enterprise / Keep 9 months       / Approx. 1 TB (decade)	 / Compliance

### 10.6 Network Equilibrium

Before segOP, every node bloats together. After segOP, the network diversifies but remains coherent: light nodes stay light, heavy nodes get paid, everyone shares the same rules.

## 11 Limitations and Future Work

Standardisation: define TLV type registry.
Miner policy: optional feerate minimum for segOP data.
Tooling: RPC & GUI support for segOP construction.
Pruning: implement -prunesegop flag for disk control.
Testing: run signet experiments on fee behaviour.
Extensions: explore Lightning-anchored retrieval oracles.

## 12 Conclusion

segOP restores honest pricing to blockspace.
It changes nothing about Bitcoin’s monetary rules—just how data is metered.
By isolating large payloads into a capped, fully-priced lane, it removes spam incentives while keeping on-chain creativity alive.

Keep the 520 B limit for signatures.
Let data live in segOP, pay full rate, and prune it later.
That’s all.

The outcome: smaller chainstate, stronger miners, faster propagation, and fairer economics - Bitcoin doing what it was designed to do.

## References

[1] P. Wuille et al., BIP-141: Segregated Witness (Consensus Layer), 2017.
[2] Bitcoin Core Source Code, MAX_SCRIPT_ELEMENT_SIZE = 520, 2012.
[3] S. Nakamoto, Bitcoin: A Peer-to-Peer Electronic Cash System, 2008.
[4] M. Murch, Understanding the Mempool and Fee Market, 2020.
[5] Defenwycke, BIP-FAIR: Fair Fee Accounting for On-Chain Data (Draft), 2025.
