/*
 *  Dune City Sound System - Programmatic Tone Generation Implementation
 *
 *  Generates PCM WAV data in-memory and converts to Mix_Chunk via Mix_LoadWAV_RW.
 */

#include <audio/sounds.h>

#include <SDL_mixer.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace DuneCitySounds {

bool masterMute = false;

namespace {

constexpr int SAMPLE_RATE = 22050;
constexpr int BITS_PER_SAMPLE = 16;
constexpr int NUM_CHANNELS = 1;

// Helper: write little-endian 16-bit value into a byte buffer
static void writeLE16(uint8_t* out, uint16_t val) {
    out[0] = static_cast<uint8_t>(val & 0xff);
    out[1] = static_cast<uint8_t>((val >> 8) & 0xff);
}

// Helper: write little-endian 32-bit value into a byte buffer
static void writeLE32(uint8_t* out, uint32_t val) {
    out[0] = static_cast<uint8_t>(val & 0xff);
    out[1] = static_cast<uint8_t>((val >> 8) & 0xff);
    out[2] = static_cast<uint8_t>((val >> 16) & 0xff);
    out[3] = static_cast<uint8_t>((val >> 24) & 0xff);
}

/**
 * Generate PCM samples for a sine wave with linear fade-out envelope.
 *
 * @param frequencyHz  Frequency in Hz
 * @param numSamples   Number of samples to generate
 * @param volume       Volume 0-128, mapped to 0-32767 for 16-bit samples
 * @return             Heap-allocated 16-bit signed PCM samples (owned by caller)
 */
static int16_t* generateSineWave(int frequencyHz, int numSamples, int volume) {
    auto* samples = new int16_t[numSamples];
    const double amplitude = (volume / 128.0) * 32767.0;
    const double twoPiF = 2.0 * M_PI * frequencyHz;
    const double invRate = 1.0 / SAMPLE_RATE;

    for (int i = 0; i < numSamples; ++i) {
        // Linear fade-out: envelope goes from 1.0 to 0.0 over the duration
        double envelope = 1.0 - (static_cast<double>(i) / numSamples);
        double sample = std::sin(twoPiF * i * invRate) * amplitude * envelope;
        samples[i] = static_cast<int16_t>(std::round(sample));
    }
    return samples;
}

/**
 * Generate PCM samples for silence.
 */
static int16_t* generateSilence(int numSamples) {
    auto* samples = new int16_t[numSamples](); // zero-initialized
    return samples;
}

/**
 * Build a Mix_Chunk from raw 16-bit PCM samples (mono, little-endian).
 *
 * Wraps the PCM data in an in-memory RIFF WAV container so SDL_mixer
 * can decode it via Mix_LoadWAV_RW.
 *
 * @param samples      16-bit PCM samples (owned, will be moved)
 * @param numSamples   Number of samples
 * @return             Mix_Chunk ready to play, or nullptr on failure
 */
static Mix_Chunk* buildChunkFromSamples(int16_t* samples, int numSamples) {
    const int bytesPerSample = BITS_PER_SAMPLE / 8;
    const int pcmByteSize = numSamples * bytesPerSample;
    const int waveHeaderSize = 44;
    const int totalSize = waveHeaderSize + pcmByteSize;

    uint8_t* wav = static_cast<uint8_t*>(SDL_malloc(totalSize));
    if (!wav) {
        delete[] samples;
        return nullptr;
    }

    // RIFF chunk
    std::memcpy(wav + 0, "RIFF", 4);
    writeLE32(wav + 4, static_cast<uint32_t>(totalSize - 8));
    std::memcpy(wav + 8, "WAVE", 4);

    // fmt  sub-chunk
    std::memcpy(wav + 12, "fmt ", 4);
    writeLE32(wav + 16, 16);                     // subchunk1 size (PCM)
    writeLE16(wav + 20, 1);                     // audio format: 1 = PCM
    writeLE16(wav + 22, NUM_CHANNELS);          // num channels
    writeLE32(wav + 24, SAMPLE_RATE);           // sample rate
    writeLE32(wav + 28, SAMPLE_RATE * NUM_CHANNELS * bytesPerSample); // byte rate
    writeLE16(wav + 32, NUM_CHANNELS * bytesPerSample); // block align
    writeLE16(wav + 34, BITS_PER_SAMPLE);       // bits per sample

    // data sub-chunk
    std::memcpy(wav + 36, "data", 4);
    writeLE32(wav + 40, static_cast<uint32_t>(pcmByteSize));

    // PCM samples — convert from 16-bit signed to little-endian bytes
    uint8_t* dataStart = wav + waveHeaderSize;
    for (int i = 0; i < numSamples; ++i) {
        writeLE16(dataStart + i * 2, static_cast<uint16_t>(samples[i] + 32768));
    }
    delete[] samples;

    SDL_RWops* rw = SDL_RWFromMem(wav, totalSize);
    if (!rw) {
        SDL_free(wav);
        return nullptr;
    }

    Mix_Chunk* chunk = Mix_LoadWAV_RW(rw, 1); // free_rw = 1 (SDL takes ownership of wav)
    if (!chunk) {
        // If Mix_LoadWAV_RW fails it frees rw but NOT wav on SDL < 2.0.20;
        // on newer SDL it owns it. Be safe and free manually.
        SDL_free(wav);
        return nullptr;
    }
    return chunk;
}

/**
 * Concatenate multiple Mix_Chunks. Returns a new chunk; caller owns it.
 */
static Mix_Chunk* concatChunks(const Mix_Chunk* a, const Mix_Chunk* b) {
    if (!a && !b) return nullptr;
    if (!a) {
        auto* out = static_cast<Mix_Chunk*>(SDL_malloc(sizeof(Mix_Chunk)));
        if (!out) return nullptr;
        out->allocated = 1;
        out->volume = b->volume;
        out->alen = b->alen;
        out->abuf = static_cast<uint8_t*>(SDL_malloc(b->alen));
        if (!out->abuf) { SDL_free(out); return nullptr; }
        std::memcpy(out->abuf, b->abuf, b->alen);
        return out;
    }
    if (!b) {
        auto* out = static_cast<Mix_Chunk*>(SDL_malloc(sizeof(Mix_Chunk)));
        if (!out) return nullptr;
        out->allocated = 1;
        out->volume = a->volume;
        out->alen = a->alen;
        out->abuf = static_cast<uint8_t*>(SDL_malloc(a->alen));
        if (!out->abuf) { SDL_free(out); return nullptr; }
        std::memcpy(out->abuf, a->abuf, a->alen);
        return out;
    }

    auto* out = static_cast<Mix_Chunk*>(SDL_malloc(sizeof(Mix_Chunk)));
    if (!out) return nullptr;
    out->allocated = 1;
    out->volume = a->volume;
    out->alen = a->alen + b->alen;
    out->abuf = static_cast<uint8_t*>(SDL_malloc(out->alen));
    if (!out->abuf) { SDL_free(out); return nullptr; }
    std::memcpy(out->abuf, a->abuf, a->alen);
    std::memcpy(out->abuf + a->alen, b->abuf, b->alen);
    return out;
}

/**
 * Allocate and fill a Mix_Chunk for a single sine tone.
 */
Mix_Chunk* GenerateTone(int frequencyHz, int durationMs, int volume) {
    if (frequencyHz <= 0 || durationMs <= 0 || volume <= 0) return nullptr;
    int numSamples = (SAMPLE_RATE * durationMs) / 1000;
    if (numSamples < 1) numSamples = 1;
    int16_t* wave = generateSineWave(frequencyHz, numSamples, volume);
    return buildChunkFromSamples(wave, numSamples);
}

/**
 * Allocate and fill a Mix_Chunk for an alternating two-tone alert.
 */
Mix_Chunk* GenerateAlertTone(int freq1, int freq2, int numBeeps,
                              int beepMs, int gapMs, int volume) {
    if (numBeeps <= 0) return nullptr;

    int beepSamples = (SAMPLE_RATE * beepMs) / 1000;
    int gapSamples  = (SAMPLE_RATE * gapMs) / 1000;

    int16_t* tone1 = generateSineWave(freq1, beepSamples, volume);
    int16_t* tone2 = generateSineWave(freq2, beepSamples, volume);
    int16_t* silence = generateSilence(gapSamples);

    Mix_Chunk* chunk1 = buildChunkFromSamples(tone1, beepSamples);
    Mix_Chunk* chunk2 = buildChunkFromSamples(tone2, beepSamples);
    Mix_Chunk* chunkGap = buildChunkFromSamples(silence, gapSamples);

    Mix_Chunk* result = nullptr;
    for (int i = 0; i < numBeeps; ++i) {
        Mix_Chunk* pair = concatChunks(chunk1, chunk2);
        Mix_Chunk* withGap = concatChunks(pair, chunkGap);
        Mix_Chunk* oldResult = result;
        result = concatChunks(oldResult, withGap);
        SDL_free(oldResult);
        SDL_free(pair);
        SDL_free(withGap);
    }

    SDL_free(chunk1);
    SDL_free(chunk2);
    SDL_free(chunkGap);
    return result;
}

/**
 * Allocate and fill a Mix_Chunk for a rising arpeggio.
 */
Mix_Chunk* GenerateArpeggio(const int* frequencies, int numNotes,
                             int noteMs, int volume) {
    if (!frequencies || numNotes <= 0) return nullptr;

    Mix_Chunk* result = nullptr;
    for (int i = 0; i < numNotes; ++i) {
        int noteSamples = (SAMPLE_RATE * noteMs) / 1000;
        int16_t* wave = generateSineWave(frequencies[i], noteSamples, volume);
        Mix_Chunk* noteChunk = buildChunkFromSamples(wave, noteSamples);
        Mix_Chunk* oldResult = result;
        result = concatChunks(oldResult, noteChunk);
        SDL_free(oldResult);
        SDL_free(noteChunk);
    }
    return result;
}

} // anonymous namespace

// ===========================================================================
// Public sound functions
// ===========================================================================

void Sound_ZoneBuilt() {
    if (masterMute) return;
    Mix_Chunk* chunk = GenerateTone(880, 100, MIX_MAX_VOLUME / 2);
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
        Mix_FreeChunk(chunk);
    }
}

void Sound_DisasterWarning() {
    if (masterMute) return;
    // Three alternating beeps: 440 Hz / 554 Hz (A4 / C#5 — tritone, unsettling)
    Mix_Chunk* chunk = GenerateAlertTone(440, 554, 3, 150, 50, MIX_MAX_VOLUME * 2 / 3);
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
        Mix_FreeChunk(chunk);
    }
}

void Sound_PowerOutage() {
    if (masterMute) return;
    // Low 220 Hz tone with longer decay — ominous hum
    Mix_Chunk* chunk = GenerateTone(220, 400, MIX_MAX_VOLUME / 2);
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
        Mix_FreeChunk(chunk);
    }
}

void Sound_Milestone() {
    if (masterMute) return;
    // C5 -> E5 -> G5 -> C6 arpeggio (major chord, triumphant)
    int notes[] = { 523, 659, 784, 1047 };
    Mix_Chunk* chunk = GenerateArpeggio(notes, 4, 120, MIX_MAX_VOLUME * 2 / 3);
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
        Mix_FreeChunk(chunk);
    }
}

void Sound_CombatNearby() {
    if (masterMute) return;
    // Low rumble: 110 Hz with slight overlay at 165 Hz (low fifth)
    Mix_Chunk* low  = GenerateTone(110, 250, MIX_MAX_VOLUME / 2);
    Mix_Chunk* mid  = GenerateTone(165, 250, MIX_MAX_VOLUME / 3);
    Mix_Chunk* combined = concatChunks(low, mid);
    Mix_Chunk* result = concatChunks(combined, mid);
    if (result) {
        Mix_PlayChannel(-1, result, 0);
        Mix_FreeChunk(result);
    }
    SDL_free(low);
    SDL_free(mid);
    SDL_free(combined);
}

void Sound_TaxCollected() {
    if (masterMute) return;
    // Pleasant two-tone ascending: 440 Hz -> 660 Hz
    int notes[] = { 440, 660 };
    Mix_Chunk* chunk = GenerateArpeggio(notes, 2, 80, MIX_MAX_VOLUME / 2);
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
        Mix_FreeChunk(chunk);
    }
}

void Sound_BudgetLow() {
    if (masterMute) return;
    // Urgent double beep at 600 Hz
    Mix_Chunk* chunk = GenerateAlertTone(600, 600, 2, 100, 50, MIX_MAX_VOLUME * 2 / 3);
    if (chunk) {
        Mix_PlayChannel(-1, chunk, 0);
        Mix_FreeChunk(chunk);
    }
}

} // namespace DuneCitySounds
