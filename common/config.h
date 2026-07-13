#pragma once

#include <array>

// Scale ..................................................................
static constexpr uint8_t kStringScaleSize = 8;
static constexpr uint8_t kStringScalesCount = 3;
static constexpr std::array<std::array<float, kStringScaleSize>, kStringScalesCount> kStringScales = {{
    { 130.81f, 196.00f, 233.08f, 261.63f, 293.66f, 311.13f, 349.23f, 392.00f }, // Amara
    { 130.81f, 164.81f, 174.61f, 196.00f, 220.00f, 261.63f, 329.63f, 349.23f }, // Oxalis
    { 130.81f, 146.83f, 155.56f, 196.00f, 233.08f, 261.63f, 293.66f, 311.13f }  // Pigmy
}};

// Reverb .................................................................
static constexpr float kReverbFeedback = .8f;
static constexpr float kReverLPFreq = 10000.f; //Hz

// MIDI ...................................................................
#if !DEBUG
#define USB_MIDI
#endif
