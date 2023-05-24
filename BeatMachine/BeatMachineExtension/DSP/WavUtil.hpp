//
//  WavUtil.hpp
//  BeatMachine
//
//  Created by Austin Kang on 5/24/23.
//
#include <fstream>

#ifndef WavUtil_h
#define WavUtil_h

struct RiffChunk {
    uint8_t riff[4];           // RIFF string
    uint32_t overall_size;     // overall size of file in bytes
    uint8_t wave[4];           // WAVE string
};

struct ChunkInfo {
    uint8_t header_name[4];    // string
    uint32_t size;     // overall size of file in bytes
};

struct FmtData {
    uint16_t format_type;    // format type
    uint16_t channels;       // number of channels
    uint32_t sample_rate;      // sampling rate (blocks per second)
    uint32_t byterate;         // SampleRate * NumChannels * BitsPerSample/8
    uint16_t block_align;    // NumChannels * BitsPerSample/8
    uint16_t bits_per_sample;// bits per sample, 8- 8bits, 16- 16 bits etc
};

std::vector<float> load_wav_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // Handle error
        return {};
    }

    // Read RIFF Chunk
    RiffChunk riffChunk;
    file.read(reinterpret_cast<char*>(&riffChunk), sizeof(RiffChunk));
    
    // Check RIFF Chunk
    if (std::strncmp((const char *)riffChunk.riff, "RIFF", 4) != 0 ||
        std::strncmp((const char *)riffChunk.wave, "WAVE", 4) != 0) {
        // This is a very basic check and will not handle all kinds of .wav files
        // In a real application, you would want to do a more thorough check
        return {};
    }
    
    // Skip chunks until we find a "fmt " or "data" chunk
    ChunkInfo chunkInfo;
    FmtData fmtData;
    
    do {
        // Read chunk ID and size
        file.read(reinterpret_cast<char*>(&chunkInfo), sizeof(chunkInfo));

        if(std::strncmp((const char *)chunkInfo.header_name, "fmt ", 4) != 0) {
            file.seekg(chunkInfo.size, std::ios::cur);  // Skip chunk
        } else {
            // Read format data
            file.read(reinterpret_cast<char*>(&fmtData), sizeof(FmtData));
        }
    } while(std::strncmp((const char *)chunkInfo.header_name, "fmt ", 4) != 0);
    
    do {
        // Read chunk ID and size
        file.read(reinterpret_cast<char*>(&chunkInfo), sizeof(chunkInfo));

        if(std::strncmp((const char *)chunkInfo.header_name, "data", 4) != 0) {
            file.seekg(chunkInfo.size, std::ios::cur);  // Skip chunk
        }
    } while(std::strncmp((const char *)chunkInfo.header_name, "data", 4) != 0);

    // Check Data Chunk
    if (std::strncmp((const char *)chunkInfo.header_name, "data", 4) != 0) {
        return {};
    }
    
    // Calculate number of samples
    size_t num_samples = chunkInfo.size / (fmtData.bits_per_sample / 8);

    // Read samples
    std::vector<float> data(num_samples);
    for (size_t i = 0; i < num_samples; ++i) {
        short sample;
        file.read(reinterpret_cast<char*>(&sample), sizeof(short));
        data[i] = sample / 32768.0f;  // Convert to [-1, 1]
    }

    return data;
}

#endif /* WavUtil_h */
