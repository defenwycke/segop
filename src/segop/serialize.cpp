// SPDX-License-Identifier: MIT
// segOP - serialization integration layer
// Author: Defenwycke (2025)

#include <segop/segop.h>
#include <hash.h>
#include <streams.h>

// Serialize segOP into a byte vector
std::vector<unsigned char> SerializeSegop(const SegopSection& s)
{
    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    s.Serialize(stream);
    return std::vector<unsigned char>(stream.begin(), stream.end());
}

// Deserialize segOP from raw bytes
SegopSection DeserializeSegop(const std::vector<unsigned char>& data)
{
    CDataStream stream(data, SER_NETWORK, PROTOCOL_VERSION);
    SegopSection s;
    s.Unserialize(stream);
    return s;
}

// Compute hash commitment for P2SOP and block commitments
uint256 ComputeSegopCommitment(const SegopSection& s)
{
    auto bytes = SerializeSegop(s);
    return Hash(bytes.begin(), bytes.end());
}
