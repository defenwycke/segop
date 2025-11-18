# SegOP RPC Helper Reference

**Hyper Hash / segOP Bitcoin Core Fork – Developer RPC Guide**  
**Last updated:** 18-Nov-2025

This guide documents the custom RPC methods introduced by the segOP-enabled Bitcoin Core fork.  
It is intended for developers, reviewers, researchers, and Bitcoin Core contributors evaluating segOP.

---

## Overview

SegOP adds a small set of RPCs that extend node functionality **without modifying any legacy RPCs**.

Goals of the segOP RPC layer:

- Provide clean access to segOP payloads (TLV sequences)
- Allow wallets to construct segOP-enabled transactions
- Expose P2SOP commitments for debugging, audits, and research
- Maintain compatibility with all standard Bitcoin Core behaviors

SegOP introduces **three categories of RPCs**:

- **Inspection RPCs** (decoding, TLV parsing)  
- **Wallet Construction RPCs** (creating segOP transactions)  
- **Utility RPCs** (commitment helpers, validation helpers)

---

# 1. Inspection RPCs

## `decodesegop "rawtxhex"`

Decode segOP payloads and TLV sequences from any raw transaction.

### Example

```
btccli decodesegop "$RAW_TX"  
```

### Returns

```
{
  "has_segop": true,
  "version": 1,
  "size": 34,
  "hex": "0109666972737420544c56010a7365636f6e6420544c56...",
  "tlv": [
    {
      "type": "0x01",
      "length": 9,
      "value_hex": "666972737420544c56",
      "text": "first TLV"
    }
  ]
}  
```

### Notes

- Works for **any** transaction — mempool or confirmed.  
- Safely handles malformed or truncated segOP payloads.  
- Returns `"has_segop": false` for non-segOP Bitcoin transactions.

---

# 2. Wallet Construction RPCs

## `segopsend "address" amount "payload" options`

Creates and funds a segOP-enabled transaction.

This is the **primary helper** for wallet users and regtest/testnet developers.

### Arguments

| Arg     | Type   | Description                        |
|---------|--------|------------------------------------|
| address | string | Destination Bitcoin address        |
| amount  | number | Amount in BTC                      |
| payload | string | Depends on encoding mode           |
| options | object | Additional parameters              |

### Options (Full List)

| Field                      | Type           | Description |
|---------------------------|----------------|-------------|
| version                   | int            | Payload version (default = 1) |
| encoding                  | string         | `"text"`, `"hex"`, `"text_multi"` |
| p2sop                     | bool           | Include P2SOP commitment output |
| texts                     | array<string>  | Required for `"text_multi"` |
| hexdata                   | string         | Required for `"hex"` encoding |
| subtract_fee_from_output  | bool           | Optional |

---

## Encoding Modes

### **1. Single-TLV text**

```
btccli segopsend "$ADDR" 0.001 "hello world" \
'{"encoding":"text","version":1,"p2sop":true}'  
```

**Result:**  
- One TLV (`0x01`)  
- UTF-8 payload  

---

### **2. Multiple TLVs (text_multi)**

```
btccli segopsend "$ADDR" 0.001 "ignored" \
'{
  "encoding":"text_multi",
  "version":1,
  "p2sop":true,
  "texts":["first TLV", "second TLV", "third TLV"]
}'  
```

**Result:**  
- Ordered TLV sequence  
- CompactSize lengths  

**Example TLV encoding:**

```
01 09 <first TLV bytes>  
01 0A <second TLV bytes>  
01 09 <third TLV bytes>  
```

---

### **3. Hex encoding**

```
btccli segopsend "$ADDR" 0.001 "" \
'{"encoding":"hex","hexdata":"deadbeef","p2sop":true}'  
```

### Returns

```
{
  "txid": "...",
  "hex": "... raw transaction ...",
  "complete": true
}  
```

---

# 3. Utility RPCs

## `getsegopdata "txid"`

Returns only the segOP payload.

```
btccli getsegopdata "$TXID"  
```

### Output

```
{
  "hex": "0109666972737420544c56...",
  "text": "first TLV"
}  
```

---

# 4. Validation & Debug RPCs

## `segopvalidate "rawtxhex"`

Runs segOP-specific validation checks:

- Version correctness  
- TLV structure  
- P2SOP commitment match  
- Length sanity checks  

*(Developer builds only)*

```
btccli segopvalidate "$RAW"  
```

---

# 5. P2SOP Commitment RPCs (if enabled)

## `getp2sop "rawtxhex"`

Returns:

- SHA256(payload)  
- Full P2SOP script  
- Commitment bytes  
- Output index  

---

**End of Document**
