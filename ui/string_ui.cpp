#include "string_ui.h"
#include "daisysp.h"
#include "config.h"

using namespace synthux;
using namespace daisy;
using namespace daisysp;

float norm(const uint8_t value) {
    return static_cast<float>(value) / 127.f;
};

void StringUI::Init(daisy::DaisySeed& hw) {
    // Callbacks ................................................
    using namespace std::placeholders;
    auto on_touch = std::bind(&StringUI::_on_pad_touch, this, _1);
    auto on_release = std::bind(&StringUI::_on_pad_release, this, _1);
    _touch.pads().SetOnTouch(on_touch);
    _touch.pads().SetOnRelease(on_release);

    _pattern_value.Set(1.f);
    _shift_value.Set(0.f);

    // Initialize MIDI //////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    #ifdef USB_MIDI
    daisy::MidiUsbHandler::Config midi_cfg;
    _midi.Init(midi_cfg);
    #endif
};

void StringUI::Process(DaisySeed& hw) {
    #ifdef USB_MIDI
    _process_midi();
    #endif

    // Process touch ////////////////////////////////////////////
    /////////////////////////////////////////////////////////////
    _touch.Process();
    _is_to_touched = _touch.pads().IsTouched(10);
    _is_ch_touched = _touch.pads().IsTouched(11);

    // Arp mode / latch ..........................................
    auto switch_value = _touch.switches().A();
    std::array<bool, kNotesCount> touched;
    for (uint8_t i = 0; i < kNotesCount; i++) {
        touched[i] = _touch.pads().IsTouched(i + kFirstNotePad);
    }
    _string.SetLatch(switch_value == daisy::Switch3::POS_UP);
    _string.SetArpOn(switch_value != daisy::Switch3::POS_DOWN);

    // Pitch (real-time) ..........................................
    _string.SetTransp(_touch.knobs().s31().Process());

    // Timbre (sample-and-hold at pluck time) .....................
    _string.SetDamping(_touch.knobs().s37().Process());
    _string.SetStructure(_touch.knobs().s32().Process());
    _string.SetBrightness(_touch.knobs().s30().Process());

    // Note randomization (arp only) ..............................
    _string.SetHumanNoteChance(_touch.knobs().s34().Process());

    // Human string chance <-> reverb mix .........................
    auto human_verb_knob_value = _touch.knobs().s35().Process();
    auto verb_stage = _verb_value.Process(human_verb_knob_value, _is_to_touched);
    auto human_string_raw = _human_string_value.Process(human_verb_knob_value, !_is_to_touched);
    _string.SetHumanStringChance(human_string_raw);
    _string.SetReverbMix(verb_stage);

    // Pattern density <-> pattern shift ..........................
    auto pattern_shift_value = _touch.knobs().s33().Process();
    auto pattern_amt = _pattern_value.Process(pattern_shift_value, !_is_to_touched);
    auto shift_amt = _shift_value.Process(pattern_shift_value, _is_to_touched);
    _string.SetPattern(pattern_amt);
    _string.SetPatternShift(shift_amt);

    // Drive / volume compensation ................................
    _string.SetDrive(_touch.knobs().s36().Process());

    hw.SetLed(_string.IsLatched());
};

void StringUI::_on_pad_touch(uint16_t pad) {
    // Scale & Tempo
    if (pad == 0) {
        if (_is_to_touched) _string.SlowDown();
        else if (_is_ch_touched) _prev_scale();
        return;
    }
    if (pad == 2) {
        if (_is_to_touched) _string.SpeedUp();
        else if (_is_ch_touched) _next_scale();
        return;
    }

    if (pad < kFirstNotePad || pad >= kFirstNotePad + kNotesCount - 1) return;
    auto note_num = pad - kFirstNotePad;
    _string.NoteOn(note_num);
};

void StringUI::_on_pad_release(uint16_t pad) {
    if (pad < kFirstNotePad || pad >= kFirstNotePad + kNotesCount) return;
    auto note_num = pad - kFirstNotePad;
    _string.NoteOff(note_num);
};

#ifdef USB_MIDI
void StringUI::_process_midi() {
    _string.ProcessClockIn(false);
    _midi.Listen();
    while(_midi.HasEvents()) {
        auto msg = _midi.PopEvent();
        switch(msg.type) {
            case SystemRealTime: {
                switch (msg.srt_type) {
                    case TimingClock: {
                        _string.ProcessClockIn(true);
                    }
                    break;
                    default: break;
                }
            }
            break;
            // Note: unlike Bass, the string engine's NoteOn/NoteOff take a
            // scale-degree index (0..7), not a chromatic MIDI note number,
            // so incoming NoteOn/NoteOff messages aren't forwarded here -
            // matching the original sketch, which had no MIDI note input at all.
            case ControlChange: {
                auto ctrl_msg = msg.AsControlChange();
                auto num = ctrl_msg.control_number;
                auto norm_value = norm(ctrl_msg.value);
                if (num == 70) { _string.SetBrightness(norm_value); }         // brightness
                else if (num == 71) { _string.SetTransp(norm_value); }       // transpose
                else if (num == 72) { _string.SetStructure(norm_value); }    // structure
                else if (num == 73) { _pattern_value.Set(norm_value); _string.SetPattern(norm_value); }        // pattern
                else if (num == 74) { _string.SetHumanNoteChance(norm_value); } // note randomization
                else if (num == 75) { _human_string_value.Set(norm_value); _string.SetHumanStringChance(norm_value); } // string randomization
                else if (num == 76) { _string.SetDamping(norm_value); }      // damping
                else if (num == 77) { _verb_value.Set(norm_value); _string.SetReverbMix(norm_value); }          // reverb
                else if (num == 78) { _shift_value.Set(norm_value); _string.SetPatternShift(norm_value); }      // pattern shift
                else if (num == 79) { _string.SetDrive(norm_value); }        // drive
                else if (num == 123) { _string.Reset(); }
            }
            break;

            default: break;
        }
    }
};
#endif
