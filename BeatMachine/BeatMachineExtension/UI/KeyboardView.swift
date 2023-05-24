import SwiftUI

struct KeyboardView: View {
    @ObservedObject var midiNote: ObservableAUParameter
    @ObservedObject var noteOn: ObservableAUParameter

    private let whiteKeys = [0, 2, 4, 5, 7, 9, 11]
    private let keyWidth: CGFloat = 20
    private let keyHeight: CGFloat = 80

    var body: some View {
        ZStack {
            HStack(spacing: 0) {
                ForEach(0..<7*6, id: \.self) { key in
                    let note = 48 + key
                    let isBlack = !whiteKeys.contains(note % 12)
                    if !isBlack {
                        WhiteKeyView(midiNote: midiNote, noteOn: noteOn, noteNumber: note)
                    }
                }
            }
            HStack(spacing: 0) {
                ForEach(0..<7*7, id: \.self) { key in
                    let note = 48 + key
                    let isBlack = !whiteKeys.contains(note % 12)
                    if isBlack {
                        BlackKeyView(midiNote: midiNote, noteOn: noteOn, noteNumber: note)
                    } else {
                        if note % 12 == 3 || note % 12 == 10 {
                            RoundedRectangle(cornerRadius: 2)
                                .fill(.gray)
                                .frame(width: 12, height: keyHeight * 0.6)
                            RoundedRectangle(cornerRadius: 2)
                                .fill(.gray)
                                .frame(width: 12, height: keyHeight * 0.6)
                        } else {
                            RoundedRectangle(cornerRadius: 2)
                                .fill(.clear)
                                .frame(width: 12, height: keyHeight * 0.6)
                        }
                    }
                }
            }
            .frame(height: keyHeight * 0.6)
            .padding(.leading, keyWidth * 0.5)
            .padding(.trailing, keyWidth * 0.5)
        }
    }
}

struct WhiteKeyView: View {
    @ObservedObject var midiNote: ObservableAUParameter
    @ObservedObject var noteOn: ObservableAUParameter
    private let keyWidth: CGFloat = 20
    private let keyHeight: CGFloat = 80
    var noteNumber: Int
    
    @State private var pressed: Float = 0
    
    func getPressState() -> Void {
        var currentIsPressed: Float = 0
        if (noteOn.value == 1 && Int(midiNote.value) == noteNumber) {
            currentIsPressed = 1
        } else if (noteOn.value == 0 && Int(midiNote.value) == noteNumber) {
            currentIsPressed = 0
        } else {
            currentIsPressed = pressed
        }
        pressed = currentIsPressed
    }
    
    var body: some View {
        RoundedRectangle(cornerRadius: 2)
            .fill(pressed == 1 ? Color.blue : Color.white)
            .frame(width: 24, height: keyHeight)
            .border(Color.black, width: 1)
            .onChange(of: noteOn.value, perform: { _ in
                getPressState()
            })
            .onChange(of: midiNote.value, perform: { _ in
                getPressState()
            })
    }
}

struct BlackKeyView: View {
    @ObservedObject var midiNote: ObservableAUParameter
    @ObservedObject var noteOn: ObservableAUParameter
    private let keyWidth: CGFloat = 20
    private let keyHeight: CGFloat = 80
    
    var noteNumber: Int
    
    @State private var pressed: Float = 0
    
    func getPressState() -> Void {
        var currentIsPressed: Float = 0
        if (noteOn.value == 1 && Int(midiNote.value) == noteNumber) {
            currentIsPressed = 1
        } else if (noteOn.value == 0 && Int(midiNote.value) == noteNumber) {
            currentIsPressed = 0
        } else {
            currentIsPressed = pressed
        }
        pressed = currentIsPressed
    }

    
    var body: some View {
        RoundedRectangle(cornerRadius: 2)
            .fill(pressed == 1 ? Color.blue : Color.black)
            .frame(width: 12, height: keyHeight * 0.6)
            .overlay(RoundedRectangle(cornerRadius: 2).stroke(Color.black, lineWidth: 1))
            .onChange(of: noteOn.value, perform: { _ in
                getPressState()
            })
            .onChange(of: midiNote.value, perform: { _ in
                getPressState()
            })
    }
}
