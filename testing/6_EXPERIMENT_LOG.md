# segOP Experiment Log #6

**Date:** 6 November 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)  

## Objective

Automate the entire segOP transaction creation and broadcast process through a single, wallet-integrated Bash workflow (`segop_send.sh`).  
This establishes the basis for a future Core RPC (`segopsend`) by replicating all required internal steps externally.

## Summary of Experiment

### 1. Script Goals

The new script replaces manual command sequences used in Experiment #5 with a fully automated process:
- Auto-detect and load wallet.
- Generate P2SOP commitment from arbitrary TLV payload.
- Build, fund, and sign transaction internally.
- Attach segOP payload post-signing.
- Broadcast and optionally mine a confirmation block.
- Print verified on-chain data including segOP field.

### 2. Implementation Details

**File:** `/root/bitcoin-segop/segop_send.sh`

**Key Additions**
| Stage | Function | Description |
|:--|:--|:--|
| 0 | Wallet management | Loads or verifies the wallet; prints summary (name, balance, txcount). |
| 1 | `segopbuildp2sop` | Computes P2SOP commitment (`534f50 || SHA256(payload)`). |
| 2 | `createrawtransaction` | Builds transaction with payment and OP_RETURN commitment. |
| 3 | `fundrawtransaction` | Selects UTXOs and sets change/fee. |
| 4 | `signrawtransactionwithwallet` | Signs all inputs via wallet keys. |
| 5 | `createsegoptx` | Attaches segOP TLV payload (`01020304` test vector). |
| 6 | `sendrawtransaction` | Broadcasts transaction to network. |
| 7 | `generatetoaddress` | (Optional) Mines a block and retrieves full TX with segOP. |

**CLI Example**
```
./segop_send.sh segoptest bcrt1qg3asjp6yvw2hwxe4e76exd52e7wlhv4r2tu5y0 0.1 01020304 mine
```

### 3. Successful Results

Broadcasted transaction:

```
TXID: 11e92311ac3f4d8f8dee7fad7902c7542241333bbc0b5d2b94078a53b84423df
```

Confirmed within block:

```
blockhash: 14d6b2b4b0a40ebbb521501e5c1242d741b55274cd0ffb600377ee67dc73385c
```

Decoded segOP field:

```
"segop": {
"version": 1,
"size": 4,
"hex": "01020304"
}
```

**Verification Summary**
- End-to-end flow stable across multiple iterations.
- Fee estimation consistent with Core defaults.
- segOP section deterministically encoded and retrievable post-mining.
- Zero parse errors or backward-compatibility regressions.
- Average transaction weight: 1.05 kWU.

## 4. Discussion

This experiment demonstrates practical wallet-layer integration without requiring internal RPCs.  
It confirms that the segOP workflow can be seamlessly embedded into user-facing APIs.

The next phase will:
- Port `segop_send.sh` logic into Core as a native RPC (`segopsend`).
- Expose higher-level arguments for TLV encoding.
- Enable GUI / RPC wallet clients to broadcast segOP transactions directly.

## 5. Notes

- Compatible with Core v30.0-based segOP build.
- All tests performed under regtest conditions with deterministic addresses.
- Payload test vector: `01020304` (TLV minimal case).
- Repository snapshot: `/src_v1` and `segop-work` branch archived.
- Future plan: Implement TLV validation rules + payload size caps at consensus level.

**Conclusion**

Experiment #6 establishes the first fully automated wallet-integrated segOP transaction pipeline.  
This marks the transition from *manual testing* to *operational usability* within Core’s ecosystem.  
The next step — implementing RPC `segopsend` — will finalize segOP’s usability for miners, nodes, and developers.

— *Defenwycke, 2025*
