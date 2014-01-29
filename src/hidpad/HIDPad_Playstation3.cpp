/*  MFiWrapper
 *  Copyright (C) 2014 - Jason Fetters
 * 
 *  MFiWrapper is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  MFiWrapper is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with MFiWrapper.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include "HIDManager.h"
#include "MFiWrapper.h"

#include "HIDPad.h"

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

HIDPad::Playstation3::Playstation3(HIDManager::Connection* aConnection) : Interface(aConnection)
{
    memset(data, 0, sizeof(data));

#ifdef IOS
    // Magic packet to start reports
    static uint8_t data[] = {0x53, 0xF4, 0x42, 0x03, 0x00, 0x00};
    HIDManager::SendPacket(device->connection, data, 6);
#endif

   // Without this the digital buttons won't be reported
   SetPlayerIndex(-1);
   
   FinalizeConnection();
}
 
void HIDPad::Playstation3::SetPlayerIndex(int32_t aIndex)
{
    Interface::SetPlayerIndex(aIndex);

    // TODO: Can this be modified to turn off motion tracking?
    static uint8_t report_buffer[] = {
        0x52, 0x01,
        0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00
    };

    report_buffer[11] = 1 << ((playerIndex % 4) + 1);
    //report_buffer[4] = motors[1] >> 8;
    //report_buffer[6] = motors[0] >> 8;
    HIDManager::SendPacket(connection, report_buffer, sizeof(report_buffer));
}

void HIDPad::Playstation3::HandlePacket(uint8_t *aData, uint16_t aSize)
{
    memcpy(data, aData, aSize);
    
    if (!stateManager)
        return;

    uint32_t buttons = data[3] | (data[4] << 8) | ((data[5] & 1) << 16);
    static const uint32_t button_mapping[17] = 
    {
        0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
        MFi_Up, MFi_Down, MFi_Left, MFi_Right,
        MFi_LeftTrigger, MFi_RightTrigger, MFi_LeftShoulder, MFi_RightShoulder,
        MFi_Y, MFi_B, MFi_A, MFi_X, 0xFFFFFFFF
    };
    
    for (int i = 0; ARRAY_SIZE(button_mapping); i ++)
        stateManager->SetButton(i, (buttons & (1 << button_mapping[i])) ? 1.0f : 0.0f);
}

const char* HIDPad::Playstation3::GetVendorName() const
{
    return "SixAxis/DualShock 3";
}
