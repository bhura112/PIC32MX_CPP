#pragma once

#include <cstdint>



struct UsbCdcAcm {

    static bool init(bool);

    static bool service(uint32_t, uint8_t);

};
