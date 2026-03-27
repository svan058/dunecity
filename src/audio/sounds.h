/*
 *  Dune City Sound System - Programmatic Tone Generation
 *
 *  Generates simple sine wave tones in-memory for city event feedback.
 *  No external audio files required for v1.
 */

#ifndef AUDIO_SOUNDS_H
#define AUDIO_SOUNDS_H

#include <SDL_mixer.h>

namespace DuneCitySounds {

/**
 * Master mute toggle. When true, all city sounds are suppressed.
 */
extern bool masterMute;

/**
 * Zone Built — short confirmation beep.
 * Frequency: 880 Hz (A5), 100ms duration, clean sine wave.
 */
void Sound_ZoneBuilt();

/**
 * Disaster Warning — distinct alert for sandstorm or sandworm attack.
 * Frequency: 440 Hz (A4) + 554 Hz alternating, 3 beeps, ominous feel.
 */
void Sound_DisasterWarning();

/**
 * Power Outage — low warning tone when zones lose power.
 * Frequency: 220 Hz (A3), 300ms duration with decay envelope.
 */
void Sound_PowerOutage();

/**
 * City Milestone — achievement fanfare when city reaches population milestone.
 * Arpeggio: C5 -> E5 -> G5 -> C6, 80ms per note.
 */
void Sound_Milestone();

/**
 * Combat Nearby — ominous rumble when military units engage near city zones.
 * Low frequency: 110 Hz with noise overlay, 200ms.
 */
void Sound_CombatNearby();

} // namespace DuneCitySounds

#endif // AUDIO_SOUNDS_H
