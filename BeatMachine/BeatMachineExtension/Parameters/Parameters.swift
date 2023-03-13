//
//  Parameters.swift
//  BeatMachineExtension
//
//  Created by Austin Kang on 2/12/23.
//

import Foundation
import AudioToolbox

let BeatMachineExtensionParameterSpecs = ParameterTreeSpec {
    ParameterGroupSpec(identifier: "global", name: "Global") {
        ParameterSpec(
            address: .gain,
            identifier: "gain",
            name: "Output Gain",
            units: .linearGain,
            valueRange: 0.0...1.0,
            defaultValue: 0.25
        )
        ParameterSpec(
            address: .isRecording,
            identifier: "isRecording",
            name: "Is Recording",
            units: .generic,
            valueRange: 0.0...1.0,
            defaultValue: 0.0
        )
        ParameterSpec(
            address: .MIDINote,
            identifier: "MIDINote",
            name: "MIDI Note",
            units: .generic,
            valueRange: 0...127,
            defaultValue: 0
        )
    }
}

extension ParameterSpec {
    init(
        address: BeatMachineExtensionParameterAddress,
        identifier: String,
        name: String,
        units: AudioUnitParameterUnit,
        valueRange: ClosedRange<AUValue>,
        defaultValue: AUValue,
        unitName: String? = nil,
        flags: AudioUnitParameterOptions = [AudioUnitParameterOptions.flag_IsWritable, AudioUnitParameterOptions.flag_IsReadable],
        valueStrings: [String]? = nil,
        dependentParameters: [NSNumber]? = nil
    ) {
        self.init(address: address.rawValue,
                  identifier: identifier,
                  name: name,
                  units: units,
                  valueRange: valueRange,
                  defaultValue: defaultValue,
                  unitName: unitName,
                  flags: flags,
                  valueStrings: valueStrings,
                  dependentParameters: dependentParameters)
    }
}
