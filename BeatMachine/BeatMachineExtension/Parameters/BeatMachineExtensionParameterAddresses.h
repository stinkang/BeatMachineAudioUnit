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
    samplingMode = 1,
    loopRecordMode = 2
};

#ifdef __cplusplus
}
#endif
