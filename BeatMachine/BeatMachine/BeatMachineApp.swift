//
//  BeatMachineApp.swift
//  BeatMachine
//
//  Created by Austin Kang on 2/12/23.
//

import CoreMIDI
import SwiftUI

@main
class BeatMachineApp: App {
    @ObservedObject private var hostModel = AudioUnitHostModel()

    required init() {}

    var body: some Scene {
        WindowGroup {
            ContentView(hostModel: hostModel)
        }
    }
}
