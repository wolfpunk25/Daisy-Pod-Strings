// SYNTHUX ACADEMY .........................................
// ARPEGGIATED STRING ......................................
#pragma once
#include <array>
#include <random>
#include <algorithm>

#include <daisysp.h>

#include "nocopy.h"
#include "config.h"

#include "synclock.h"
#include "trigger.h"
#include "cpattern.h"
#include "arp.h"
#include "scale.h"
#include "vox.h"
#include "xfade.h"
#include "latch.h"

namespace synthux {

class String {
public:
  static constexpr uint8_t kNotesCount = 8;

  String();
  ~String() {}

  void Init(const float sample_rate, const float buffer_size);

  void SetTempo(const float tempo) { _clock.SetTempo(tempo); }
  void SpeedUp() {
    _tempo = std::min(_tempo + .05f, 1.f);
    SetTempo(_tempo);
  }
  void SlowDown() {
    _tempo = std::max(_tempo - .05f, .05f);
    SetTempo(_tempo);
  }
  void ProcessClockIn(const bool state) { _clock.Process(state); }

  void SetArpOn(const bool value) { _is_arp_on = value; }

  bool IsLatched() { return _latch.on(); }
  void SetLatch(const bool new_latch);

  void NoteOn(const uint8_t note_num);
  void NoteOff(const uint8_t note_num);

  void Reset();

  uint8_t ScalesCount() { return _scale.ScalesCount(); }
  void SetScaleIndex(const uint8_t index) { _scale.SetScaleIndex(index); }

  // Real-time, applied every call (not gated to note-on).
  void SetTransp(const float value) { _vox.SetMult(_scale.TransMult(value)); }

  // Sample-and-hold at pluck time, exactly like the original sketch:
  // these only reach the voice inside NoteOn/the arp note-on callback.
  void SetBrightness(const float value) { _brightness = value; }
  void SetStructure(const float value) { _structure = value; }
  void SetDamping(const float value) { _damping = value * .7f; }

  void SetHumanNoteChance(const float value) {
    _human_note_chance = static_cast<uint8_t>(daisysp::fmap(value, 0.f, 100.f));
  }
  void SetHumanStringChance(const float value) {
    _human_string_chance = static_cast<uint8_t>(daisysp::fmap(value, 0.f, 100.f));
  }

  void SetPattern(const float value) { _pattern.SetOnsets(value); }
  void SetPatternShift(const float value) { _pattern.SetShift(value); }

  void SetReverbMix(const float value) { _xfade.SetStage(value); }

  void SetDrive(const float value) {
    _drive.SetDrive(0.2f + value * .3f);
    _volume = 1.f - value * 0.6f;
    _volume *= _volume;
  }

  void Process(float **out, size_t size);

private:
  NOCOPY(String)

  void _on_clock_tick();
  void _on_arp_note_on(uint8_t num, uint8_t vel);
  void _on_arp_note_off(uint8_t num) {}

  void _on_latch_note_on(uint8_t num);
  void _on_latch_note_off(uint8_t num);

  float _humanized_note_freq(uint8_t note);
  void _humanize_and_apply();

  static constexpr uint8_t kPPQN = 48;
  static constexpr uint8_t kPPQNExtern = 24;

  Scale _scale;
  Vox   _vox;
  SynClock _clock;
  Trigger  _trigger;
  CPattern _pattern;
  Arp<kNotesCount, 4> _arp;
  daisysp::Overdrive _drive;
  daisysp::ReverbSc  _reverb;
  XFade _xfade;
  Latch<kNotesCount> _latch;

  std::default_random_engine _rand_engine;
  std::uniform_int_distribution<uint8_t> _dice;

  std::array<bool, kNotesCount> _note_hold;
  std::array<bool, kNotesCount> _note_on;
  std::array<float, 2> _reverb_in;
  std::array<float, 2> _reverb_out;
  std::array<float, 2> _bus;

  float _tempo;
  float _brightness;
  float _structure;
  float _damping;
  uint8_t _human_note_chance;
  uint8_t _human_string_chance;
  float _volume;
  bool _is_arp_on;
};

};
