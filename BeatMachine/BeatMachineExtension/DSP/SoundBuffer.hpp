//
//  SoundBuffer.h
//  BeatMachine
//
//  Created by Austin Kang on 5/23/23.
//

#import <AudioToolbox/AudioToolbox.h>
#import <CoreMIDI/CoreMIDI.h>
#import <Accelerate/Accelerate.h>
#import <algorithm>
#import <vector>
#import <span>
#include <iostream>
#include <unordered_map>
#include <set>

#ifndef SoundBuffer_h
#define SoundBuffer_h

class SoundBuffer {
private:
    AudioBufferList* bufferList;
    int sampleIndex;
    int playIndex;
    
public:
    void initialize() {
        bufferList = new AudioBufferList;
        bufferList->mNumberBuffers = 1;
        
        AudioBuffer& buffer = bufferList->mBuffers[0];
        buffer.mNumberChannels = 1;
        buffer.mDataByteSize = 1024 * sizeof(float);
        buffer.mData = new float[441000];
        
        sampleIndex = 0;
        playIndex = 0;
    }
    
    ~SoundBuffer() {
        // Make sure to free the memory that we've allocated
        delete [] static_cast<float*>(bufferList->mBuffers[0].mData);
        delete bufferList;
    }
    
    // Records a single sample into the buffer and advances the sampleIndex
    void recordSample(float sample) {
        float* recordBufferChannel = static_cast<float*>(bufferList->mBuffers[0].mData) + sampleIndex;
        std::memcpy(recordBufferChannel, &sample, sizeof(float));
        sampleIndex += 1;
    }
    
    // Plays a single sample from the buffer and advances the playIndex
    float playSample() {
        float gain = 1.0f;
        if (playIndex < crossfadeSamples) {
            // Fade in
            gain = playIndex / crossfadeSamples;
        }
        float sample = *((float*)bufferList->mBuffers[0].mData + playIndex) * gain;
        playIndex += 1;
        return sample;
    }
    
    // Resets the buffer for new recording
    void reset() {
        sampleIndex = 0;
        playIndex = 0;
    }
    
    double crossfadeSamples = 0.01 * 44100.0;
};

#endif /* SoundBuffer_h */
