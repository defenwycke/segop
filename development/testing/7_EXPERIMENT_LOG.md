# segOP Experiment Log #7

**Date:** 7 November 2025  
**Author:** Defenwycke  
**Project:** segOP (Segregated OP_RETURN)  

## Objective

Validate the stability and determinism of the new segop_send.sh automation script over multiple runs, and confirm end-to-end block inclusion of diverse payloads.
Establish a clear baseline for transition toward a native Core RPC (segopsend).

## Summary of Experiment
### 1. Test Scope

Execute the full automated workflow multiple times with different payloads.

Confirm payload integrity, correct P2SOP commitment, and successful mining.

Observe legacy node compatibility and mempool behaviour.

### 2. Environment

Bitcoin Core v30.0 (segOP branch segop-v2)

Wallet: segoptest (regtest)

Scripts: /root/bitcoin-segop/segop_send.sh (latest revision)

Nodes: 1 segOP node + 1 baseline node

### 3. Procedure

Rebooted regtest environment to ensure clean chain state.

Ran ten sequential payload tests (01020300 → 01020309).

Each run invoked:

```
./segop_send.sh segoptest bcrt1qg3asjp6yvw2hwxe4e76exd52e7wlhv4r2tu5y0 0.05 <payload> mine
```

Verified each resulting TXID and block inclusion using:

```
bitcoin-cli -regtest getrawtransaction <txid> 1
```

Compared segop field outputs and validated SHA256(commitment) matches.

### 4. Results
Payload	TXID (truncated)	Block Height	Status
01020300	c41d…7b9a	#106	Confirmed
01020301	b77e…1e22	#107	Confirmed
…	…	…	…
01020309	4a8f…d951	#115	Confirmed

All ten transactions confirmed with correct segOP TLV fields:

```
"segop": {
  "version": 1,
  "size": 4,
  "hex": "01020309"
}
```

### 5. Verification Summary

Deterministic encoding across iterations — no hash drift.

Each block contained valid P2SOP commitments.

No mempool rejections or policy conflicts.

Legacy node ignored segOP data silently, confirming backward-compatibility.

## Discussion

This experiment proves the repeatability and stability of the automated shell workflow.
It also confirms that segOP payloads incur full 4 WU/byte cost and coexist with SegWit transactions without witness discount leakage.

Next phase: port script logic into Core’s RPC layer for native execution and wallet integration.

**Notes**
- All tests under regtest with default 1 MB blocks.
- Fee rate ≈ 1 sat/vB; average TX weight ≈ 1.07 kWU.
- No pruning or relay anomalies observed.

## Conclusion

Experiment #7 confirms the segOP automated pipeline is deterministic, miner-compatible, and stable under multi-transaction load.
The groundwork for the internal RPC (segopsend) is validated.

— Defenwycke, 2025
