//
//  BeatMachineExtensionParameterAddresses.h
//  BeatMachineExtension
//
//  Created by Austin Kang on 2/12/23.
//

#pragma once

#include <AudioToolbox/AUParameters.h>

#ifdef __cplusplus
namespace BeatMachineExtensionParameterAddress {
#endif

typedef NS_ENUM(AUParameterAddress, BeatMachineExtensionParameterAddress) {
    gain = 0,
    isRecording = 1,
    MIDINote = 2,
    MIDINoteOff = 3
};

#ifdef __cplusplus
}
#endif
