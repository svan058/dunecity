/*
 *  Dune City Sound System - Programmatic Tone Generation
 *
 *  Generates simple sine wave tones in-memory for city event feedback.
 *  No external audio files required for v1.
 */

#ifndef AUDIO_SOUNDS_H
#define AUDIO_SOUNDS_H

#include <misc/SDL2pp.h>

#include <SDL_mixer.h>

namespace DuneCitySounds {

/**
 * Master mute toggle. When true, all city sounds are suppressed.
 */
extern bool masterMute;

sdl2::mix_chunk_ptr CreateZoneBuiltChunk();
sdl2::mix_chunk_ptr CreateDisasterWarningChunk();
sdl2::mix_chunk_ptr CreatePowerOutageChunk();
sdl2::mix_chunk_ptr CreateMilestoneChunk();
sdl2::mix_chunk_ptr CreateTaxCollectedChunk();
sdl2::mix_chunk_ptr CreateBudgetLowChunk();

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

/**
 * Tax Collected — pleasant rising tone when city tax is collected.
 * Two-tone ascending: 440 Hz -> 660 Hz, 80ms each.
 */
void Sound_TaxCollected();

/**
 * Budget Low Warning — urgent double beep when city budget is critical.
 * Two quick beeps at 600 Hz, 100ms each with short gap.
 */
void Sound_BudgetLow();

} // namespace DuneCitySounds

#endif // AUDIO_SOUNDS_H
