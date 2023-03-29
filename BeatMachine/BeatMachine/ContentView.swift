//
//  ContentView.swift
//  BeatMachine
//
//  Created by Austin Kang on 2/12/23.
//

import AudioToolbox
import SwiftUI

struct ContentView: View {
    @ObservedObject var hostModel: AudioUnitHostModel
    @State var filename = "Filename"
    @State var newFileName = ""
    @State var fileURL : URL?
    @State var showFileChooser = false
    var margin = 10.0
    var doubleMargin: Double {
        margin * 2.0
    }
    
    var body: some View {
        VStack() {
            Text("\(hostModel.viewModel.title )")
                .textSelection(.enabled)
                .padding()
            VStack(alignment: .center) {
                if let viewController = hostModel.viewModel.viewController {
                    AUViewControllerUI(viewController: viewController)
                        .padding(margin)
                } else {
                    VStack() {
                        Text(hostModel.viewModel.message)
                            .padding()
                    }
                    .frame(minWidth: 400, minHeight: 200)
                }
            }
            .padding(doubleMargin)
            
            if hostModel.viewModel.showAudioControls {
                HStack {
                    Spacer()
                    HStack {
                      Text(filename)
                      Button("Select File")
                      {
                        let panel = NSOpenPanel()
                        panel.allowsMultipleSelection = false
                        panel.canChooseDirectories = false
                        if panel.runModal() == .OK {
                            self.filename = panel.url?.lastPathComponent ?? "<none>"
                            self.fileURL = panel.url
                            hostModel.setNewFile(url: panel.url)
                        }
                      }
                    }
                    Spacer()
                    Text("Audio Playback")
                    Button {
                        hostModel.isPlaying ? hostModel.stopPlaying() : hostModel.startPlaying()
                    } label: {
                        Text(hostModel.isPlaying ? "Stop" : "Play")
                    }
                    Spacer()
                    TextField("File Name: ", text: $newFileName)
                    Button {
                        hostModel.handleRecording(fileName: newFileName)
                    } label: {
                        Text(hostModel.isRecording ? "Stop" : "Record")
                    }
                    Spacer()
                }
            }
            if hostModel.viewModel.showMIDIContols {
                Text("MIDI Input: Enabled")
            }
            Spacer()
                .frame(height: margin)
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView(hostModel: AudioUnitHostModel())
    }
}
