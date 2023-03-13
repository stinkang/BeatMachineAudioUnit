//
//  IsRecordingView.swift
//  BeatMachineExtension
//
//  Created by Austin Kang on 3/6/23.
//

import SwiftUI

/// A SwiftUI  container which is bound to an ObservableAUParameter
struct IsRecordingView: View {
    @ObservedObject var param: ObservableAUParameter
    
    var specifier: String {
        switch param.unit {
        case .midiNoteNumber:
            return "%.0f"
        default:
            return "%.2f"
        }
    }
    
    var body: some View {
        VStack {
            HStack {
                Text("Mapping Mode")
                Image(systemName: "circle.fill")
                    .foregroundColor(param.value == 1.0 ? .red : .gray)
                    .onTapGesture {
                        param.value = abs(param.value - 1.0)
                    }
            }
        }
        .padding()
    }
}
