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

#import "BeatMachineExtension-Swift.h"
#import "BeatMachineExtensionParameterAddresses.h"

/*
 BeatMachineExtensionDSPKernel
 As a non-ObjC class, this is safe to use from render thread.
 */
class BeatMachineExtensionDSPKernel {
public:
    std::unordered_map<UInt32, AudioBufferList*> soundBuffers;
    bool isRecording = false;
    UInt64 currentNoteTime = 0;
    int RECORD_NOTE = 0x0;
    std::set<int> currentNotes;
    int sampleIndexes [128];
    int playIndexes [128];
    
    void initialize(int inputChannelCount, int outputChannelCount, double inSampleRate) {
        mSampleRate = inSampleRate;
        for (UInt32 i = 0; i < 128; i++) {
            AudioBufferList* bufferList = new AudioBufferList;
            bufferList->mNumberBuffers = 1;

            AudioBuffer& buffer = bufferList->mBuffers[0];
            buffer.mNumberChannels = 1;
            buffer.mDataByteSize = 1024 * sizeof(float);
            buffer.mData = new float[441000];

            this->soundBuffers.insert(std::make_pair(i, bufferList));
        }
    }
    
    void deInitialize() {
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
    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case BeatMachineExtensionParameterAddress::gain:
                mGain = value;
                break;
        }
    }
    
    AUValue getParameter(AUParameterAddress address) {
        // Return the goal. It is not thread safe to return the ramping value.
        
        switch (address) {
            case BeatMachineExtensionParameterAddress::gain:
                return (AUValue)mGain;
                
            default: return 0.f;
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
                
                // Do your sample by sample dsp here...
                
                if (this->isRecording) {
                    outputBuffers[channel][frameIndex] = inputBuffers[channel][frameIndex] * 1.0 * mGain;
                    if (this->currentNotes.size() != 0) {
                        for (auto currentNote = currentNotes.begin(); currentNote != currentNotes.end(); ++currentNote) {
                            recordInputToSoundBuffer(inputBuffers, channel, frameIndex, *currentNote);
                        }
                    }
                } else {
                    // reset outputBuffers
                    outputBuffers[channel][frameIndex] = 0;
                    
                    if (this->currentNotes.size() != 0) {
                        for (auto currentNote = currentNotes.begin(); currentNote != currentNotes.end(); ++currentNote) {
                            outputBuffers[channel][frameIndex] += (*((float*)this->soundBuffers[*currentNote]->mBuffers[0].mData + this->playIndexes[*currentNote])) * 1.0 * mGain;
                            this->playIndexes[*currentNote] += 1;
                        }
                    }
                }
            }
        }
    }
    
    void recordInputToSoundBuffer(std::span<float const*> inputBuffers, UInt32 channel, UInt32 frameIndex, int currentNote) {
        float* recordBufferChannel = (float*)this->soundBuffers[currentNote]->mBuffers[0].mData + this->sampleIndexes[currentNote];

        std::memcpy(recordBufferChannel, &inputBuffers[channel][frameIndex], sizeof(float));
        this->sampleIndexes[currentNote] += 1;
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
                    thisObject->isRecording = true;
                } else {
                    thisObject->currentNotes.insert(noteNumber);
                }
            } else if (message.channelVoice2.status == kMIDICVStatusNoteOff) {
                UInt32 noteNumber = message.channelVoice2.note.number;
                if (noteNumber == thisObject->RECORD_NOTE) {
                    thisObject->isRecording = false;
                } else {
                    thisObject->currentNotes.erase(noteNumber);
                    thisObject->sampleIndexes[noteNumber] = 0;
                    thisObject->playIndexes[noteNumber] = 0;
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
    
    // MARK: - Member Variables
    AUHostMusicalContextBlock mMusicalContextBlock;
    
    double mSampleRate = 44100.0;
    double mGain = 1.0;
    double mNoteEnvelope = 0.0;
    
    bool mBypassed = false;
    AUAudioFrameCount mMaxFramesToRender = 1024;
};
