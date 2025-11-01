## Experiment specification for implementing segOP on bitcoin

### Goals

1. Integrate segOP after SegWit within the coinbase.
2. Include RPC for utility
3. Prove non-segOP nodes ignore segOP and are unaffected. (Backward compatability).
4. Prove segOP nodes send/receive segOP and can extract payloads.
5. Implement pruning at different levels.
6. Send genesis segOP tx to Satoshi and extract segOP payload.
6. Model mass segOP tx's to prove economics for network and miners.
7. Forecast blockchain storage implications.
8. Publish white paper.


### Tasks
1. Develop TLV format for segOP.
2. Develop flags and functions for segOP.
3. Integrate segOp into coinbase.
4. Mine segOP tx on segOP node. (Reg test)
5. Send segOP tx from segOP node to non-segOP node. (Reg test)
6. Extract segOP payload from non-segOP node. (Reg test)
7. Implement fee weights and lane rules.
8. Implement pruning.
9. Check policies.
10. Send genesis tx and extract payload. (Main net)
11. Build modal for proving economics.
12. Build modal for storage forecast.
13. Clean code.
14. Write white paper and publish.

