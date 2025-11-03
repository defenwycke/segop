# Hyper Hash Network  
### A Decentralised Mining, Node, and Data Architecture for Bitcoin  
**Version:** v0.1 — November 2025  
**Author:** Defenwycke  

## Abstract  

Bitcoin mining and node operation have become increasingly centralised due to infrastructure cost, latency asymmetry, and uneven fee economics.  
Hyper Hash Network introduces a layered system designed to restore decentralisation and fairness:  

- **Hyper Core** — an orchestrator for pool and solo mining built atop Bitcoin Core and Stratum v2.  
- **Hyper Treasury** — a Lightning-based financial layer that collects, distributes, and records all rewards.  
- **Hyper Nodes** — community-run relay and mining nodes with optional dark-mode operation for private mining.  
- **Oracle Nodes** — community-run archival peers providing paid access to full-chain and segOP data.  

All components remain interoperable with Bitcoin Core and Lightning Network.  
The network’s 1 % pool fee is divided equally between the Treasury and eligible Nodes, creating a transparent, self-funded, and incentive-balanced ecosystem.

## 1. Motivation  

Bitcoin’s design depends on decentralised validation and block propagation.  
However, consolidation of hash power into a few datacentres, unequal bandwidth, and reliance on witness-discounted storage have weakened that principle.  

- Miners increasingly depend on third-party pools for co-ordination.  
- Node operation provides no tangible reward, discouraging participation.  
- Large arbitrary data (“inscriptions”) exploit SegWit’s weight discount, inflating blocks without fair fees.  

Hyper Hash seeks to counter these trends through three guiding principles:

1. **Fair economics** — miners, nodes, and archivists share value proportionally.  
2. **Low-latency propagation** — geographically distributed nodes relay tips and blocks.  
3. **Transparent governance** — all flows visible and signed on-chain or via Lightning.  

## 2. System Overview  

```
     ┌──────────────────────────────┐
     │          Hyper Core          │
     │   Mining Orchestrator + API  │
     └──────────────────────────────┘
                  │
        ┌────────────────────┐
        │   Hyper Treasury    │
        │  Payouts • Ledgers  │
        └────────────────────┘
                  │
    ┌────────────────────────────────┐
    │            Hyper Nodes          │
    │  Relay • Template • Dark Mode  │
    └────────────────────────────────┘
                  │
      ┌────────────────────────────┐
      │        Oracle Nodes        │
      │  Archive • segOP • API402  │
      └────────────────────────────┘
```

Each layer is independent yet interoperable:

- **Core** connects miners and nodes.  
- **Treasury** processes fees via Lightning.  
- **Nodes** handle block propagation and local mining.  
- **Oracles** preserve full blockchain data and serve it via pay-per-call APIs.  

## 3. Economic Model  

Mining under Hyper Hash operates through two parallel modes — **pool mining** and **solo mining** — both sustaining the network via a uniform 1 % network fee.

### Pool Mining  

Pool mining incurs a fixed **1 % network fee**, deducted from the total block reward before distribution.  

#### Reward Distribution  

| Share | Recipient | Function |
|--------|------------|-----------|
| **0.5 %** | **Node Network (Hyper + Oracle Nodes)** | Equally distributed among all eligible nodes maintaining uptime and Lightning connectivity. |
| **0.5 %** | **Hyper Treasury** | Maintains multisig wallets, Lightning liquidity, and governance operations. |
| **99 %** | **Contributing Miners** | Shared proportionally among all miners who submitted valid shares to the winning block. The *transaction-fee portion* of the block is awarded in full to the **winning miner** who solved the block. |

Mining rewards are distributed through a **share-based system**.  
Each miner submitting valid work during a round receives a payout proportional to their contributed hashrate.  
The miner who discovers the valid block header additionally receives the entire transaction-fee component of that block, incentivising block discovery while maintaining fairness across contributors.

This model ensures:
- Fair compensation for every participating miner.  
- Transparent accounting through Treasury-signed ledgers.  
- Continuous funding for node and oracle infrastructure.

### Solo Mining  

Solo miners connect directly to **Hyper Core** or their own **Hyper Node**.  
They also pay the same **1 % network fee**, ensuring economic consistency across the network.  

#### Reward Distribution  

| Share | Recipient | Function |
|--------|------------|-----------|
| **0.5 %** | **Node Network (Hyper + Oracle Nodes)** | Equal distribution to all eligible nodes for maintaining network propagation and relay services. |
| **0.5 %** | **Hyper Treasury** | Maintains liquidity, ledger publication, and protocol governance. |
| **99 %** | **Solo Miner** | Retains the full block subsidy and all transaction fees (minus the 1 % network fee). |

This maintains fee fairness between pool and solo mining while allowing independent operators to mine privately without penalty.

### Node Network (Hyper + Oracle Nodes)  

The combined **Node Network** — encompassing both **Hyper Nodes** and **Oracle Nodes** — is rewarded through the 0.5 % pool allocation described above.  
This amount is collected automatically by the Treasury and distributed equally among all nodes that meet eligibility requirements for the current epoch.

Node rewards are independent of hashrate and compensate operators for:
- Decentralised relay infrastructure and block propagation.  
- Maintaining uptime, latency mapping, and tip submission.  
- Providing archival and segOP storage through Oracle operations.  

Nodes running in **Dark Mode** (private mining mode) remain fully eligible for rewards.  
Even when restricted to local miners, they still broadcast headers, blocks, and heartbeats to Core, strengthening network coverage.

This design ensures:
- Reliable, incentivised global relay coverage.  
- Equal treatment for public and private operators.  
- Sustainable infrastructure independent of mining revenue.

### Oracle Revenue Contribution  

In addition to their share of node rewards, **Oracle Nodes contribute 1 % of all Lightning API revenue** to the Treasury.  
This links off-chain data services directly to the same economic feedback loop that funds node and governance operations.

### Eligibility  

Eligibility for infrastructure rewards requires:
- Continuous uptime for a full epoch (≈ 7 days)  
- An open Lightning channel to the Treasury  
- Valid signed heartbeats within each 24-hour period  

Payouts occur automatically via **Lightning keysend**, recorded as JSON-signed ledgers publicly verifiable on-chain or through Treasury endpoints.

## 4. Network Operation  

### 4.1 Hyper Core  

Core acts as the coordination point for:
- Node registration and heartbeat verification  
- Tip and block aggregation for latency mapping  
- Payout eligibility reporting to Treasury  

It does **not** dictate policy or require updates; all changes are additive and backward-compatible.

### 4.2 Hyper Nodes  
Nodes connect miners to the network, propagating blocks and tips while maintaining local autonomy.

**Dark Mode** allows a node to restrict access to its own miners only, yet still participate in propagation and rewards.  
This design keeps private miners private while strengthening global relay density.

### 4.3 Hyper Treasury  

Operates as a cluster of multisig Lightning wallets performing:
- Fee collection (from pool + Oracle)  
- Equal node payout distribution  
- Ledger signing and publication  
- Governance voting through threshold signatures  

### 4.4 Oracle Nodes  

Full archival peers storing the entire Bitcoin blockchain, including segOP payloads.  
Provide Lightning-authenticated (402) APIs for:  
- Transaction and block retrieval  
- segOP TLV payload decoding  
- Historical analytics  

They also function as free P2P peers for other nodes or Bitcoin Core clients requiring initial block download or verification — preserving open access while monetising higher-level data services.

## 5. segOP — Segregated OP_RETURN  

**segOP** defines a structured, fee-fair data section inside Bitcoin transactions.  
It introduces a segregated TLV-encoded payload paid at full weight (4 WU / byte) and prunable independently of witness data.  

### Benefits  

| Problem | segOP Solution |
|----------|----------------|
| Witness misuse for arbitrary data | Dedicated, full-fee data lane |
| Unstructured on-chain metadata | TLV + Merkle commitments |
| Indefinite data retention | Optional pruning windows |
| Quantum-era signatures | Payload space for quantum-secure proofs |

By combining segOP with Oracle Nodes, Hyper Hash enables **paid archival storage** without bloating Bitcoin’s consensus state.

## 6. Decentralisation and Fairness  

- **Open participation:** any operator can deploy a node or oracle with identical code.  
- **Voluntary upgrades:** backward compatibility preserved indefinitely.  
- **Public accountability:** all ledgers, fees, and governance actions cryptographically signed.  
- **Geographic equity:** node-based routing ensures miners connect to the lowest-latency peer.  

This restores the original Bitcoin ethos — independent actors collaborating through incentives, not hierarchy.

## 7. Compatibility  

Hyper Hash builds atop, not beside, Bitcoin:  

| Component | Compatible With |
|------------|----------------|
| Node Relay | Bitcoin Core P2P / Stratum v2 |
| Treasury | Lightning Network (BOLT 11 / keysend) |
| Oracle | Bitcoin Core peers + REST API mirror |
| segOP | Bitcoin v30 + soft-fork safe extension |

Bitcoin Core users may connect to Oracle Nodes directly for bootstrap or verification, ensuring that Hyper Hash reinforces—not replaces—the Bitcoin network.

## 8. Governance  

The Treasury multisig quorum manages:
- Epoch length and reward intervals  
- Treasury key rotation  
- Protocol version tagging  

No changes may invalidate existing nodes or ledgers.  
All governance proposals are published through signed JSON objects in the Treasury API.

## 9. Future Work  

1. **gRPC Transport** – low-latency binary messaging for node heartbeats.  
2. **segOP BIP Submission** – formal proposal to Bitcoin Core developers.  
3. **Latency Overlay Routing** – peer-to-peer path optimisation among nodes.  
4. **UI Integration** – public dashboard visualising pool, treasury, and node metrics.  
5. **Lightning Yield Tracking** – reinvestment analytics for compounding treasury balance.  

## 10. Conclusion  

Hyper Hash Network re-balances the economics of participation in Bitcoin.  
It transforms passive node operation into a rewarded public good,  
channels mining fees into transparent Lightning flows,  
and introduces a sustainable model for paid on-chain data.  

Every node, miner, and oracle strengthens the network — not through trust,  
but through aligned incentives, open code, and fair fees.

*© 2025 Hyper Hash Network — Authored by Defenwycke — CC BY-SA 4.0*

