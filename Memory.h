template <bool Writeable>
class MemoryBank {
public:
    uint8_t Read(Addr address);
    void Write(Addr address, uint8_t value);
};

using ROMBank = MemoryBank<false>;
using RAMBank = MemoryBank<true>;