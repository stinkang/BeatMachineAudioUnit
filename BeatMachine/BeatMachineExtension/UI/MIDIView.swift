//
//  MIDIView.swift
//  BeatMachineExtension
//
//  Created by Austin Kang on 3/6/23.
//

import SwiftUI

struct MIDIView: View {
    @ObservedObject var midiNote: ObservableAUParameter
    @ObservedObject var noteOn: ObservableAUParameter
    
    var body: some View {
        Text("MIDI Note: " + String(midiNote.value) + (noteOn.value == 1 ? " On" : " Off"))
    }
}
