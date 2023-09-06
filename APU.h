class APU {
public:
    APU();
    ~APU();

    void PowerOn();
    void Reset();

    void Step();

    uint8_t Read(uint16_t address);
    void Write(uint16_t address, uint8_t value);

    enum APUAddr : Addr {
        APUAddr_SQ1_VOL = 0x4000,
        APUAddr_SQ1_SWEEP = 0x4001,
        APUAddr_SQ1_LO = 0x4002,
        APUAddr_SQ1_HI = 0x4003,
        APUAddr_SQ2_VOL = 0x4004,
        APUAddr_SQ2_SWEEP = 0x4005,
        APUAddr_SQ2_LO = 0x4006,
        APUAddr_SQ2_HI = 0x4007,
        APUAddr_TRI_LINEAR = 0x4008,
        APUAddr_TRI_LO = 0x400A,
        APUAddr_TRI_HI = 0x400B,
        APUAddr_NOISE_VOL = 0x400C,
        APUAddr_NOISE_LO = 0x400E,
        APUAddr_NOISE_HI = 0x400F,
        APUAddr_DMC_FREQ = 0x4010,
        APUAddr_DMC_RAW = 0x4011,
        APUAddr_DMC_START = 0x4012,
        APUAddr_DMC_LEN = 0x4013,
        APUAddr_STATUS = 0x4015,
        APUAddr_FRAME = 0x4017
    };

};