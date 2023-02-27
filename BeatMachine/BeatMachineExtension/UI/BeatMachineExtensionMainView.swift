//
//  BeatMachineExtensionMainView.swift
//  BeatMachineExtension
//
//  Created by Austin Kang on 2/12/23.
//

import SwiftUI

struct BeatMachineExtensionMainView: View {
    var parameterTree: ObservableAUParameterGroup

    var body: some View {
        ParameterSlider(param: parameterTree.global.gain)
    }
}
