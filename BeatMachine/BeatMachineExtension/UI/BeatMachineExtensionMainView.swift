//
//  BeatMachineExtensionMainView.swift
//  BeatMachineExtension
//
//  Created by Austin Kang on 2/12/23.
//

import SwiftUI

struct BeatMachineExtensionMainView: View {
    var parameterTree: ObservableAUParameterGroup
    
    init(parameterTree: ObservableAUParameterGroup) {
        self.parameterTree = parameterTree
        print(parameterTree)
    }

    var body: some View {
        VStack(spacing: 10) {
            ParameterSlider(param: parameterTree.global.gain)
            IsRecordingView(param: parameterTree.global.samplingMode)
            IsRecordingView(param: parameterTree.global.loopRecordMode)
            //KeyboardView(midiNote: parameterTree.global.MIDINote, noteOn: parameterTree.global.NoteOn)
        }
    }
}
