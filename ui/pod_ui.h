#pragma once

#include <array>
#include <cstdint>

#include "daisy_pod.h"
#include "mvalue.h"
#include "../string/string.h"

namespace synthux {

class PodUI {
  public:
    PodUI(daisy::DaisyPod& hardware, String& string);

    void Init();
    void Process();

  private:
    enum class PlayMode : uint8_t {
        Direct,
        Arpeggiator,
        Latch,
        Count,
    };

    enum class Page : uint8_t {
        Tone,
        Body,
        Rhythm,
        Human,
        Space,
        Count,
    };

    enum Parameter : uint8_t {
        Brightness,
        Structure,
        Damping,
        Drive,
        Pattern,
        PatternShift,
        HumanNote,
        HumanString,
        Reverb,
        Transpose,
        ParameterCount,
    };

    void ProcessControls();
    void ProcessMidi();
    void ProcessParameters();
    void UpdateLeds();
    void SetMode(PlayMode mode);
    void NextScale();
    void NextPage();
    void NoteOn(uint8_t midi_note, uint8_t velocity);
    void NoteOff(uint8_t midi_note);

    static constexpr uint8_t kMidiChannel = 0;
    static constexpr uint8_t kFirstMidiNote = 60;
    static constexpr uint8_t kScaleSelectCc = 20;
    static constexpr uint16_t kScaleFlashFrames = 125;

    daisy::DaisyPod& _hw;
    String& _string;
    std::array<MValue, ParameterCount> _values;
    PlayMode _mode;
    Page _page;
    uint8_t _scale_index;
    uint16_t _scale_flash_frames;
};

} // namespace synthux
