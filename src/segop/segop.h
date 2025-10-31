// SPDX-License-Identifier: MIT
// segOP header – soft-fork-safe segregated data section
// Author: Defenwycke (2025)

#ifndef BITCOIN_SEGOP_H
#define BITCOIN_SEGOP_H

#include <serialize.h>
#include <streams.h>
#include <vector>
#include <string>
#include <stdint.h>

static constexpr unsigned char SEGOP_FLAG = 0x02;
static constexpr unsigned char SEGOP_MAGIC[3] = {'S','O','P'};

// ────────────────────────────────────────────────
// TLV Record
// ────────────────────────────────────────────────
struct SegopTLV {
    uint8_t type{0};
    uint16_t length{0};
    std::vector<unsigned char> value;

    template <typename Stream>
    void Serialize(Stream& s) const {
        ::Serialize(s, type);
        ::Serialize(s, length);
        s.write((const char*)value.data(), value.size());
    }

    template <typename Stream>
    void Unserialize(Stream& s) {
        ::Unserialize(s, type);
        ::Unserialize(s, length);
        value.resize(length);
        s.read((char*)value.data(), length);
    }
};

// ────────────────────────────────────────────────
// segOP Section
// ────────────────────────────────────────────────
struct SegopSection {
    uint8_t version{1};
    std::vector<SegopTLV> entries;

    template <typename Stream>
    void Serialize(Stream& s) const {
        // Magic + version + count + TLVs
        s.write((const char*)SEGOP_MAGIC, 3);
        ::Serialize(s, version);

        uint8_t count = entries.size();
        ::Serialize(s, count);

        for (const auto& tlv : entries)
            tlv.Serialize(s);
    }

    template <typename Stream>
    void Unserialize(Stream& s) {
        unsigned char magic[3];
        s.read((char*)magic, 3);
        if (memcmp(magic, SEGOP_MAGIC, 3) != 0) {
            throw std::ios_base::failure("Invalid segOP magic");
        }

        ::Unserialize(s, version);
        uint8_t count{0};
        ::Unserialize(s, count);

        entries.resize(count);
        for (auto& tlv : entries)
            tlv.Unserialize(s);
    }
};

// ────────────────────────────────────────────────
// Helper functions
// ────────────────────────────────────────────────
inline std::string SegopToString(const SegopSection& segop)
{
    std::string s = "SOPv" + std::to_string(segop.version) + " {";
    for (const auto& tlv : segop.entries) {
        s += " T" + std::to_string(tlv.type) + ":";
        s += std::string(tlv.value.begin(), tlv.value.end());
    }
    s += " }";
    return s;
}

#endif // BITCOIN_SEGOP_H
