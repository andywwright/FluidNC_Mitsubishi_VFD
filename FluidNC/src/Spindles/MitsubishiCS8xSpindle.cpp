// Copyright (c) 2023 -	Andy Wright
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

/*
    MitsubishiCS8xSpindle.cpp

    This is for the MitsubishiCS8x VFD based spindle via RS485 Modbus.

                         WARNING!!!!
    VFDs are very dangerous. They have high voltages and are very powerful
    Remove power before changing bits.
*/

#include "MitsubishiCS8xSpindle.h"

namespace Spindles {
    MitsubishiCS8x::MitsubishiCS8x() : VFD() {}

    void MitsubishiCS8x::direction_command(SpindleState mode, ModbusCommand& data) {
        data.tx_length = 6;
        data.rx_length = 6;

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x00;  
        data.msg[3] = 0x08;
        data.msg[4] = 0x00;

        switch (mode) {
            case SpindleState::Cw:
                data.msg[5] = 0x02;
                break;
            case SpindleState::Ccw:
                data.msg[5] = 0x04;
                break;
            default:  // SpindleState::Disable
                data.msg[5] = 0x00;
                break;
        }
    }

    void MitsubishiCS8x::set_speed_command(uint32_t dev_speed, ModbusCommand& data) {

        data.tx_length = 6;
        data.rx_length = 6;

        // [01][06][00][0D][00][00]
        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x00; 
        data.msg[3] = 0x0D;
        data.msg[4] = dev_speed >> 8;
        data.msg[5] = dev_speed & 0xFF;
    }

    VFD::response_parser MitsubishiCS8x::initialization_sequence(int index, ModbusCommand& data) {
        
        if (index == -1) {
            data.tx_length = 6;
            data.rx_length = 7;

            // Send: [01][03][03][E9][00][02]
            data.msg[1] = 0x03;  
            data.msg[2] = 0x03;  
            data.msg[3] = 0xE9;
            data.msg[4] = 0x00;  
            data.msg[5] = 0x02;

            //  Recv: <01><03><04><01><2C><9C><40>

            return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {

                uint16_t maxRPM = (response[5] << 8) | response[6];

                if (vfd->_speeds.size() == 0) {
                    vfd->shelfSpeeds(maxRPM / 4, maxRPM);
                }

                vfd->setupSpeeds(maxRPM);               // The speed is given directly in RPM
                vfd->_slop                      = 300;  // 300 RPM

                static_cast<MitsubishiCS8x*>(vfd)->_maxRPM = uint32_t(maxRPM);

                log_info("MitsubishiCS8x spindle initialized at " << maxRPM << " RPM");

                return true;
            };
        } else {
            return nullptr;
        }
    }

    VFD::response_parser MitsubishiCS8x::get_current_speed(ModbusCommand& data) {
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: [01][03][00][0D][00][01]
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x00; 
        data.msg[3] = 0x0D;
        data.msg[4] = 0x00; 
        data.msg[5] = 0x01;

        //  Recv: <01><03><02><00><00>
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            vfd->_sync_dev_speed = (uint16_t(response[3]) << 8) | uint16_t(response[4]);
            return true;
        };
    }

    VFD::response_parser MitsubishiCS8x::get_current_direction(ModbusCommand& data) {
        data.tx_length = 6;
        data.rx_length = 5;

        // [01][03][00][08][00][01]
        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x00; 
        data.msg[3] = 0x08;
        data.msg[4] = 0x00;
        data.msg[5] = 0x01;

        // Receive: <01><03><02><00><00>

        // TODO: What are we going to do with this? Update vfd state?
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool { return true; };
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<MitsubishiCS8x> registration("MitsubishiCS8x");
    }
}
