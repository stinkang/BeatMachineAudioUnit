//
//  MIDIView.swift
//  BeatMachineExtension
//
//  Created by Austin Kang on 3/6/23.
//

import SwiftUI

struct MIDIView: View {
    @ObservedObject var param: ObservableAUParameter
    
    var body: some View {
        Text("Current MIDI Note: " + String(param.value))
    }
}
