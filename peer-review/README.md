# segOP Peer Review Pack [DRAFT]

This directory contains everything needed to review, test, and reason about segOP (Segregated OP_RETURN):

- A Bitcoin Core fork with segOP integrated.
- A minimal wallet / UI for interactive testing.
- A Docker-based stack to spin everything up quickly.
- The specification, BIP draft, simulations, and supporting docs.

## Layout

### docs/

- `segop-spec.md` — canonical technical specification of segOP.
- `segop-bip.md` — BIP-style draft text (what will be sent to bitcoin-dev).
- `segop-whitepaper.md` — higher-level narrative + design background.
- `segop-simulations.md` — summaries of backtests / simulations.
- `faq.md` — anticipated questions and concise answers.

### segop-bitcoin/

- `README.md` — how to build and run the segOP Bitcoin Core fork.
- `rpc/` — RPC documentation and examples for constructing/inspecting segOP txs.
- `scripts/` — helper scripts to spin up regtest and send example segOP txs.
- `tests/` — functional tests that exercise segOP behaviour.
- `notes/` — extra design commentary for reviewers (non-normative).

### segop-wallet/

- `README.md` — how to run the reference wallet / UI.
- `docs/` — wallet architecture and usage.
- `src/` — wallet source code (segOP-aware client).
- `examples/` — example calls and (optionally) screenshots.

### stack/

- `README.md` — Docker/compose based quickstart.
- `docker-compose.yml` — one command stack launch.
- `Dockerfile.core`, `Dockerfile.wallet` — build images for node and wallet.
- `scripts/` — automation for regtest init + demo transactions.

## Quickstart (developer / reviewer)

### 1. Run the full stack (Docker)

The easiest way to see segOP in action is via Docker:

```
cd peer-review/stack
docker compose up --build
```

This will:

- Build and run a segOP-enabled Bitcoin Core node on regtest.
- Optionally run the segOP wallet/UI against that node.
- Run init scripts to:
  - Mine a baseline set of regtest blocks.
  - Fund a wallet.
  - Broadcast at least one example segOP transaction.

See `stack/README.md` for exact ports, credentials, and what to look at.

### 2. Run segOP node directly (no Docker)

If you prefer to build and run the segOP fork manually:

Clone the segOP Core fork:

```
TODO: replace with actual URL / branch

git clone <SEGOP_BITCOIN_CORE_REPO_URL> bitcoin-segop
cd bitcoin-segop
git checkout <SEGOP_BRANCH>
```

Build and run on regtest as usual.

Use the RPCs described in `segop-bitcoin/rpc/segop-rpcs.md` to:

- Construct a segOP transaction.
- Broadcast it.
- Inspect segOP data and per-block statistics.

Where to start reading

If you’re new to segOP and want a sensible path:

- `docs/segop-spec.md` — precise protocol definition.
- `docs/segop-bip.md` — BIP-style narrative + deployment.
- `segop-bitcoin/rpc/segop-rpcs.md` — hands-on: how to build and inspect segOP txs.
- `docs/segop-simulations.md` — how segOP affects blockspace and miner fee dynamics.

### Feedback & discussion

Open issues or PRs against this repository, or reach out via the usual Bitcoin development channels once the BIP draft is posted.

This pack is intentionally self-contained: if you can build Bitcoin Core, you should be able to build and test segOP.
