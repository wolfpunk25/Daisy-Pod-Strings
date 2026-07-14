#include "string.h"
#include <functional>

using namespace synthux;

String::String():
_trigger              { Trigger(kPPQN) },
_dice                 { std::uniform_int_distribution<uint8_t>(0, 100) },
_tempo                { .45f },
_brightness           { 0.f },
_structure            { 0.f },
_damping              { 0.f },
_human_note_chance    { 0 },
_human_string_chance  { 0 },
_volume               { 1.f },
_is_arp_on            { false }
{
  _note_on.fill(false);
  _note_hold.fill(false);
  _reverb_in.fill(0.f);
  _reverb_out.fill(0.f);
  _bus.fill(0.f);
};

void String::Init(const float sample_rate, const float buffer_size) {
  using namespace std::placeholders;

  _clock.Init(1e6 * buffer_size / sample_rate, kPPQNExtern, kPPQN);
  auto on_clock = std::bind(&String::_on_clock_tick, this);
  _clock.SetOnTick(on_clock);

  auto on_latch_note_on = std::bind(&String::_on_latch_note_on, this, _1);
  auto on_latch_note_off = std::bind(&String::_on_latch_note_off, this, _1);
  _latch.set_on_note_on(on_latch_note_on);
  _latch.set_on_note_off(on_latch_note_off);

  auto on_arp_note_on = std::bind(&String::_on_arp_note_on, this, _1, _2);
  auto on_arp_note_off = std::bind(&String::_on_arp_note_off, this, _1);
  _arp.SetOnNoteOn(on_arp_note_on);
  _arp.SetOnNoteOff(on_arp_note_off);
  
  _arp.SetDirection(ArpDirection::fwd);
  _arp.SetRandChance(0);
  _arp.SetAsPlayed(true);

  _vox.Init(sample_rate);
  _drive.Init();

  _reverb.Init(sample_rate);
  _reverb.SetFeedback(kReverbFeedback);
  _reverb.SetLpFreq(kReverLPFreq);

  SetTempo(_tempo);
};

void String::SetLatch(const bool on) {
    _latch.set_on(on);
    if (!_arp.HasNote()) Reset();
};

void String::NoteOn(const uint8_t num) {
  if (!_is_arp_on) {
    _humanize_and_apply();
    _vox.NoteOn(_scale.FreqAt(num), 1.f);
    return;
  } 

  _latch.note_on(num);

  if (_arp.HasNote()) {
    if (!_clock.IsRunning()) _clock.Run();
  }
  else {
    Reset();
  }
};

void String::NoteOff(const uint8_t num) {
  _latch.note_off(num);

  if (!_arp.HasNote()) {
    Reset();
  }
};

void String::Reset() {
  _clock.Stop();
  _trigger.Reset();
  _pattern.Reset();
  _arp.Clear();
};

void String::Process(float **out, size_t size) {
  _clock.Tick();
  for (size_t i = 0; i < size; i++) {
    _bus[0] = _bus[1] = _drive.Process(_vox.Process()) * _volume;
    _xfade.Process(0, 0, _bus[0], _bus[1], _reverb_in[0], _reverb_in[1]);
    _reverb.Process(_reverb_in[0], _reverb_in[1], &(_reverb_out[0]), &(_reverb_out[1]));
    out[0][i] = daisysp::SoftLimit(_bus[0] + _reverb_out[0]) * .75f;
    out[1][i] = daisysp::SoftLimit(_bus[1] + _reverb_out[1]) * .75f;
  }
};

void String::_on_clock_tick() {
  if (_trigger.Tick() && _pattern.Tick()) {
    _arp.Trigger();
  }
};

void String::_on_latch_note_on(uint8_t num) 
{ 
  _arp.NoteOn(num, 127); 
}
void String::_on_latch_note_off(uint8_t num) 
{ 
  _arp.NoteOff(num); 
}

void String::_on_arp_note_on(uint8_t num, uint8_t vel) {
  auto freq = _is_arp_on ? _humanized_note_freq(num) : _scale.FreqAt(num);
  _humanize_and_apply();
  _vox.NoteOn(freq, 1.f);
};

float String::_humanized_note_freq(uint8_t note) {
  auto freq = _scale.FreqAt(note);
  if (_human_note_chance <= 2) return freq;

  auto human_note_chance_dice = _dice(_rand_engine);
  auto note_dice = _dice(_rand_engine);
  auto octave_dice = _dice(_rand_engine);

  if (_human_note_chance < 33) {
    if (human_note_chance_dice < _human_note_chance) {
      return (octave_dice < 50) ? freq * .5f : freq * 2.f;
    }
    return freq;
  }
  else if (_human_note_chance < 66) {
    if (human_note_chance_dice < _human_note_chance) {
      if (note_dice < 50) freq = _scale.Random();
      if (octave_dice < 25) return freq * .5f;
      else if (octave_dice > 75) return freq * 2.f;
      else return freq;
    }
    return freq;
  }
  else {
    if (note_dice < _human_note_chance) freq = _scale.Random();
    if (octave_dice < 20) return freq * 2.f;
    else if (octave_dice > 80) return freq * .5f;
    return freq; // 60% of freq passes through unchanged
  }
};

void String::_humanize_and_apply() {
  if (_human_string_chance > 2) {
    auto chance_dice = _dice(_rand_engine);
    if (chance_dice < _human_string_chance) {
      auto bright_dice = _dice(_rand_engine);
      auto structure_dice = _dice(_rand_engine);
      auto damping_dice = _dice(_rand_engine);

      auto bright_delta = bright_dice * 0.002f;
      _brightness = std::min(_brightness + bright_delta, 1.f);

      auto struct_delta = structure_dice * 0.002f;
      _structure = std::min(_structure + struct_delta, 1.f);

      auto damping_delta = damping_dice * 0.004f;
      _damping = std::min(_damping - 0.004f + damping_delta, 8.f);
    }
  }
  _vox.SetBrightness(_brightness);
  _vox.SetStructure(_structure);
  _vox.SetDamping(_damping);
};
