#include "pod_ui.h"

using namespace daisy;
using namespace synthux;

namespace {

struct Rgb {
    float r;
    float g;
    float b;
};

constexpr std::array<Rgb, 3> kModeColors {{
    {0.05f, 0.85f, 0.10f}, // direct: green
    {0.05f, 0.25f, 1.00f}, // arpeggiator: blue
    {0.85f, 0.05f, 0.75f}, // latch: magenta
}};

constexpr std::array<Rgb, 5> kPageColors {{
    {1.00f, 0.30f, 0.02f}, // tone: orange
    {0.90f, 0.03f, 0.03f}, // body: red
    {0.05f, 0.80f, 0.30f}, // rhythm: green
    {0.10f, 0.75f, 1.00f}, // human: cyan
    {0.55f, 0.10f, 1.00f}, // space: violet
}};

constexpr std::array<Rgb, 3> kScaleColors {{
    {1.00f, 0.65f, 0.02f}, // Amara
    {0.05f, 1.00f, 0.35f}, // Oxalis
    {0.20f, 0.45f, 1.00f}, // Pigmy
}};

constexpr float Norm(uint8_t value)
{
    return static_cast<float>(value) / 127.f;
}

} // namespace

PodUI::PodUI(DaisyPod& hardware, String& string)
: _hw {hardware},
  _string {string},
  _mode {PlayMode::Direct},
  _page {Page::Tone},
  _scale_index {0},
  _scale_flash_frames {0}
{
}

void PodUI::Init()
{
    // Friendly musical defaults. Each paged control uses soft takeover, so
    // changing pages never makes a parameter jump to the physical knob.
    _values[Brightness].Set(0.65f);
    _values[Structure].Set(0.45f);
    _values[Damping].Set(0.50f);
    _values[Drive].Set(0.15f);
    _values[Pattern].Set(0.65f);
    _values[PatternShift].Set(0.00f);
    _values[HumanNote].Set(0.10f);
    _values[HumanString].Set(0.15f);
    _values[Reverb].Set(0.25f);
    _values[Transpose].Set(0.50f);

    SetMode(_mode);
    _string.SetScaleIndex(_scale_index);
    _hw.midi.StartReceive();
    ProcessParameters();
    UpdateLeds();
}

void PodUI::Process()
{
    ProcessControls();
    ProcessMidi();
    ProcessParameters();
    UpdateLeds();
}

void PodUI::ProcessControls()
{
    _hw.ProcessAllControls();

    if(_hw.button1.RisingEdge())
    {
        auto next = static_cast<uint8_t>(_mode) + 1;
        if(next >= static_cast<uint8_t>(PlayMode::Count))
            next = 0;
        SetMode(static_cast<PlayMode>(next));
    }

    if(_hw.button2.RisingEdge())
        NextScale();

    if(_hw.encoder.RisingEdge())
        NextPage();

    const int32_t increment = _hw.encoder.Increment();
    if(increment > 0)
    {
        for(int32_t i = 0; i < increment; ++i)
            _string.SpeedUp();
    }
    else if(increment < 0)
    {
        for(int32_t i = 0; i > increment; --i)
            _string.SlowDown();
    }
}

void PodUI::ProcessMidi()
{
    _hw.midi.Listen();
    while(_hw.midi.HasEvents())
    {
        auto event = _hw.midi.PopEvent();
        switch(event.type)
        {
            case daisy::NoteOn:
            {
                const auto note = event.AsNoteOn();
                if(note.channel != kMidiChannel)
                    break;
                if(note.velocity == 0)
                    NoteOff(note.note);
                else
                    NoteOn(note.note, note.velocity);
                break;
            }
            case daisy::NoteOff:
            {
                const auto note = event.AsNoteOff();
                if(note.channel == kMidiChannel)
                    NoteOff(note.note);
                break;
            }
            case ControlChange:
            {
                const auto cc = event.AsControlChange();
                if(cc.channel != kMidiChannel)
                    break;
                const float value = Norm(cc.value);
                switch(cc.control_number)
                {
                    case 70: _values[Brightness].Set(value); break;
                    case 71: _values[Transpose].Set(value); break;
                    case 72: _values[Structure].Set(value); break;
                    case 73: _values[Pattern].Set(value); break;
                    case 74: _values[HumanNote].Set(value); break;
                    case 75: _values[HumanString].Set(value); break;
                    case 76: _values[Damping].Set(value); break;
                    case 77: _values[Reverb].Set(value); break;
                    case 78: _values[PatternShift].Set(value); break;
                    case 79: _values[Drive].Set(value); break;
                    case 123: _string.Reset(); break;
                    default: break;
                }
                break;
            }
            case SystemRealTime:
                if(event.srt_type == TimingClock)
                {
                    _string.ProcessClockIn(true);
                    _string.ProcessClockIn(false);
                }
                break;
            case ChannelMode:
                _string.Reset();
                break;
            default: break;
        }
    }
}

void PodUI::ProcessParameters()
{
    const float knob1 = _hw.GetKnobValue(DaisyPod::KNOB_1);
    const float knob2 = _hw.GetKnobValue(DaisyPod::KNOB_2);
    const auto page = static_cast<uint8_t>(_page);

    for(uint8_t parameter = 0; parameter < ParameterCount; ++parameter)
    {
        const bool first_knob = (parameter % 2) == 0;
        const bool active = (parameter / 2) == page;
        _values[parameter].Process(first_knob ? knob1 : knob2, active);
    }

    _string.SetBrightness(_values[Brightness].Value());
    _string.SetStructure(_values[Structure].Value());
    _string.SetDamping(_values[Damping].Value());
    _string.SetDrive(_values[Drive].Value());
    _string.SetPattern(_values[Pattern].Value());
    _string.SetPatternShift(_values[PatternShift].Value());
    _string.SetHumanNoteChance(_values[HumanNote].Value());
    _string.SetHumanStringChance(_values[HumanString].Value());
    _string.SetReverbMix(_values[Reverb].Value());
    _string.SetTransp(_values[Transpose].Value());
}

void PodUI::UpdateLeds()
{
    const auto mode_color = kModeColors[static_cast<uint8_t>(_mode)];
    _hw.led1.Set(mode_color.r, mode_color.g, mode_color.b);

    Rgb second = kPageColors[static_cast<uint8_t>(_page)];
    if(_scale_flash_frames > 0)
    {
        second = kScaleColors[_scale_index];
        --_scale_flash_frames;
    }
    _hw.led2.Set(second.r, second.g, second.b);
    _hw.UpdateLeds();
}

void PodUI::SetMode(PlayMode mode)
{
    _mode = mode;
    _string.SetLatch(_mode == PlayMode::Latch);
    _string.SetArpOn(_mode != PlayMode::Direct);
}

void PodUI::NextScale()
{
    _scale_index = (_scale_index + 1) % _string.ScalesCount();
    _string.SetScaleIndex(_scale_index);
    _scale_flash_frames = kScaleFlashFrames;
}

void PodUI::NextPage()
{
    auto next = static_cast<uint8_t>(_page) + 1;
    if(next >= static_cast<uint8_t>(Page::Count))
        next = 0;
    _page = static_cast<Page>(next);
}

void PodUI::NoteOn(uint8_t midi_note, uint8_t velocity)
{
    if(velocity == 0 || midi_note < kFirstMidiNote
       || midi_note >= kFirstMidiNote + String::kNotesCount)
        return;
    _string.NoteOn(midi_note - kFirstMidiNote);
}

void PodUI::NoteOff(uint8_t midi_note)
{
    if(midi_note < kFirstMidiNote
       || midi_note >= kFirstMidiNote + String::kNotesCount)
        return;
    _string.NoteOff(midi_note - kFirstMidiNote);
}
