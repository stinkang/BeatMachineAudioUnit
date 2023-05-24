//
//  BeatMachineExtensionDSPKernel.hpp
//  BeatMachineExtension
//
//  Created by Austin Kang on 2/12/23.
//

#pragma once

#import <AudioToolbox/AudioToolbox.h>
#import <CoreMIDI/CoreMIDI.h>
#import <Accelerate/Accelerate.h>
#import <algorithm>
#import <vector>
#import <span>
#include <iostream>
#include <unordered_map>
#include <set>
#include "WavUtil.hpp"
#include <fstream>
#include "SoundBuffer.hpp"

#import "BeatMachineExtension-Swift.h"
#import "BeatMachineExtensionParameterAddresses.h"

/*
 BeatMachineExtensionDSPKernel
 As a non-ObjC class, this is safe to use from render thread.
 */
class BeatMachineExtensionDSPKernel {
private:
    AUHostMusicalContextBlock mMusicalContextBlock;
    
    // configuration parameters
    double mSampleRate = 44100.0;
    double mGain = 1.0;
    double mNoteEnvelope = 0.0;
    double crossfadeSamples = 0.01 * 44100.0;
    bool mBypassed = false;
    AUAudioFrameCount mMaxFramesToRender = 1024;
    static const int bufferCount = 128; //128 MIDI notes
    
    // sampling member variables
    std::array<SoundBuffer, bufferCount> soundBuffers;
    float samplingMode = 0.0;
    int MIDINote = 0;
    int RECORD_NOTE = 0x0;
    int MUTE_NOTE = 0x1;
    bool isMuted = false;
    std::set<int> currentNotes;
    std::unordered_map<AUParameterAddress, AUParameter*> paramRefs;
    
    // looping member variables
    int loopBufferSize;        // Size of our loop buffer (in samples)
    float* loopBuffer;         // The loop buffer itself
    int loopSampleIndex;           // Current position in the loop buffer
    bool loopRecordMode;           // Whether we're currently recording
    double beatsPerBar;        // Number of beats per bar (usually 4)
    double tempo;              // The tempo (in BPM)
    double loopLengthBars;     // The length of the loop (in bars)
    
    // metronome member variables
    double metronomeFrequency;  // Frequency of the metronome click (in Hz)
    double metronomeDuration;   // Duration of the metronome click (in seconds)
    int metronomeSampleIndex;   // Current position in the metronome click
    int metronomeSamplesPerBeat;// Number of samples per beat
    std::vector<float> clickSound;
    
public:
    void initialize(int inputChannelCount, int outputChannelCount, double inSampleRate) {
        mSampleRate = inSampleRate;
        // Initialize all soundBuffers
        for (auto &buffer : soundBuffers) {
            buffer.initialize();
        }
        
        // initialize loop variables
        beatsPerBar = 4.0;
        loopLengthBars = 4.0;
        tempo = 144.0;  // Default to 120 BPM. You may want to make this configurable.

        // Calculate how many samples make up our loop
        loopBufferSize = (int)((mSampleRate * 60.0 / tempo) * beatsPerBar * loopLengthBars);

        // Allocate the loop buffer
        loopBuffer = new float[loopBufferSize];
        
        // initialize metronome variables
        metronomeFrequency = 1000.0;  // 1000 Hz
        metronomeDuration = 0.1;     // 100 ms
        metronomeSampleIndex = 0;
        metronomeSamplesPerBeat = (int)(mSampleRate * 60.0 / tempo);
        
        NSString *path = [[NSBundle mainBundle] pathForResource:@"click" ofType:@"wav"];
        std::string pathStr = [path UTF8String];
        clickSound = load_wav_file(pathStr);
    }
    
    void deInitialize() {
        delete[] loopBuffer;
    }
    
    // MARK: - Bypass
    bool isBypassed() {
        return mBypassed;
    }
    
    void setBypass(bool shouldBypass) {
        mBypassed = shouldBypass;
    }
    
    // MARK: - Parameter Getter / Setter
    // Add a case for each parameter in BeatMachineExtensionParameterAddresses.h
    // so this is used when we set the parameter from the UI, such as with a ParameterSlider. We update the kernel's parameter.
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case BeatMachineExtensionParameterAddress::gain:
                mGain = value;
                break;
            case BeatMachineExtensionParameterAddress::samplingMode:
                samplingMode = value;
                break;
            case BeatMachineExtensionParameterAddress::loopRecordMode:
                loopRecordMode = value;
                break;
        }
    }
    
    AUValue getParameter(AUParameterAddress address) {
        // Return the goal. It is not thread safe to return the ramping value.
        
        switch (address) {
            case BeatMachineExtensionParameterAddress::gain:
                return (AUValue)mGain;
                break;
            case BeatMachineExtensionParameterAddress::samplingMode:
                return (AUValue)samplingMode;
                break;
            case BeatMachineExtensionParameterAddress::loopRecordMode:
                return (AUValue)loopRecordMode;
                break;

            default: return 0.f;
        }
    }
    
    void addParameterRef(AUParameter *param) {
        paramRefs[param.address] = param;
    }
    
    
    // we use this to set the parameter from WITHIN the kernel, so that the AUParameterTree's
    // implementorValueObserver picks up on the change. We're changing the C++ object that it's listening for.
    // note how paramRefs is an std::unordered_map<AUParameterAddress, AUParameter*>
    void setParameterRef(AUParameterAddress address, AUValue value) {
        if (paramRefs.find(address) == paramRefs.end()) {
          // error
        } else {
            paramRefs[address].value = value;
        }
    }
    
    // MARK: - Max Frames
    AUAudioFrameCount maximumFramesToRender() const {
        return mMaxFramesToRender;
    }
    
    void setMaximumFramesToRender(const AUAudioFrameCount &maxFrames) {
        mMaxFramesToRender = maxFrames;
    }
    
    // MARK: - Musical Context
    void setMusicalContextBlock(AUHostMusicalContextBlock contextBlock) {
        mMusicalContextBlock = contextBlock;
    }
    
    // MARK: - MIDI Protocol
    MIDIProtocolID AudioUnitMIDIProtocol() const {
        return kMIDIProtocol_2_0;
    }
    
    /**
     MARK: - Internal Process
     
     This function does the core siginal processing.
     Do your custom DSP here.
     */
    void process(std::span<float const*> inputBuffers, std::span<float *> outputBuffers, AUEventSampleTime bufferStartTime, AUAudioFrameCount frameCount) {
        /*
         Note: For an Audio Unit with 'n' input channels to 'n' output channels, remove the assert below and
         modify the check in [BeatMachineExtensionAudioUnit allocateRenderResourcesAndReturnError]
         */
        assert(inputBuffers.size() == outputBuffers.size());
    
        // Use this to get Musical context info from the Plugin Host,
        // Replace nullptr with &memberVariable according to the AUHostMusicalContextBlock function signature
        if (mMusicalContextBlock) {
            mMusicalContextBlock(nullptr /* currentTempo */,
                                 nullptr /* timeSignatureNumerator */,
                                 nullptr /* timeSignatureDenominator */,
                                 nullptr /* currentBeatPosition */,
                                 nullptr /* sampleOffsetToNextBeat */,
                                 nullptr /* currentMeasureDownbeatPosition */);
        }
        
        // Perform per sample dsp on the incoming float in before assigning it to out
        for (UInt32 channel = 0; channel < inputBuffers.size(); ++channel) {
            for (UInt32 frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
                if (samplingMode == 1.0) {
                    outputBuffers[channel][frameIndex] = inputBuffers[channel][frameIndex] * 1.0 * mGain;
                    if (this->currentNotes.size() != 0) {
                        for (auto currentNote = currentNotes.begin(); currentNote != currentNotes.end(); ++currentNote) {
                            this->soundBuffers[*currentNote].recordSample(inputBuffers[channel][frameIndex]);
                        }
                    }
                } else {
                    // reset outputBuffers
                    outputBuffers[channel][frameIndex] = 0;
                    if (this->currentNotes.size() != 0) {
                        for (auto currentNote = currentNotes.begin(); currentNote != currentNotes.end(); ++currentNote) {
                            outputBuffers[channel][frameIndex] += this->soundBuffers[*currentNote].playSample() * mGain;
                        }
                    }
                    if (this->isMuted) {
                        outputBuffers[channel][frameIndex] = 0;
                    }
                    
                    if (loopRecordMode == 1.0) {
                        // Check if it's time for a metronome click
                        if (metronomeSampleIndex % metronomeSamplesPerBeat == 0) {
                            metronomeSampleIndex = 0;
                        }
                        
                        // Apply the metronome click to the output
                        if (metronomeSampleIndex < clickSound.size()) {
                            float metronomeClick = clickSound[metronomeSampleIndex];
                            outputBuffers[channel][frameIndex] += metronomeClick;
                        }
                        metronomeSampleIndex++;
                    }
                }
            }
        }
    }
    
    void handleOneEvent(AUEventSampleTime now, AURenderEvent const *event) {
        switch (event->head.eventType) {
            case AURenderEventParameter: {
                handleParameterEvent(now, event->parameter);
                break;
            }
                
            case AURenderEventMIDIEventList: {
                handleMIDIEventList(now, &event->MIDIEventsList);
                break;
            }

            default:
                break;
        }
    }
    
    void handleParameterEvent(AUEventSampleTime now, AUParameterEvent const& parameterEvent) {
        // Implement handling incoming Parameter events as needed
    }
    
    void handleMIDIEventList(AUEventSampleTime now, AUMIDIEventList const* midiEvent) {
        auto visitor = [] (void* context, MIDITimeStamp timeStamp, MIDIUniversalMessage message) {
            auto thisObject = static_cast<BeatMachineExtensionDSPKernel *>(context);

            if (message.channelVoice2.status == kMIDICVStatusNoteOn) {
                UInt32 noteNumber = message.channelVoice2.note.number;
                if (noteNumber == thisObject->RECORD_NOTE) {
                    thisObject->setParameterRef(BeatMachineExtensionParameterAddress::samplingMode, 1.0);
                } else if (noteNumber == thisObject->MUTE_NOTE) {
                    thisObject->isMuted = true;
                } else {
                    thisObject->currentNotes.insert(noteNumber);
                }
            } else if (message.channelVoice2.status == kMIDICVStatusNoteOff) {
                UInt32 noteNumber = message.channelVoice2.note.number;
                if (noteNumber == thisObject->RECORD_NOTE) {
                    thisObject->setParameterRef(BeatMachineExtensionParameterAddress::samplingMode, 0.0);
                } else if (noteNumber == thisObject->MUTE_NOTE) {
                    thisObject->isMuted = false;
                } else {
                    thisObject->currentNotes.erase(noteNumber);
                    thisObject->soundBuffers[noteNumber].reset();
                }
            }
        };
        
        MIDIEventListForEachEvent(&midiEvent->eventList, visitor, this);
    }
    
    void handleMIDI2VoiceMessage(const struct MIDIUniversalMessage& message) {
        //const auto& note = message.channelVoice2.note;
        
        switch (message.channelVoice2.status) {
            case kMIDICVStatusNoteOff: {
                mNoteEnvelope = 0.0;
            }
                break;
                
            case kMIDICVStatusNoteOn: {
                mNoteEnvelope = 1.0;
            }
                break;
                
            default:
                break;
        }
    }
};
