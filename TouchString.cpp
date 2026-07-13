#include "daisy_seed.h"
#include "touch/touch.h"
#include "string/string.h"
#include "ui/string_ui.h"
#include "log.h"

using namespace daisy;
using namespace synthux;

DaisySeed hw;

Touch touch;
String engine;
StringUI ui(touch, engine);


void AudioCallback(
	AudioHandle::InputBuffer in, 
	AudioHandle::OutputBuffer out, 
	size_t size) {
	engine.Process(out, size);
};

int main(void) {
	hw.Init();
	hw.SetAudioBlockSize(4);
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	#if DEBUG
	HW::hw().setHW(&hw);
	HW::hw().startLog();
	#endif

	touch.Init(hw);
	engine.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
	ui.Init(hw);

	hw.StartAudio(AudioCallback);

	while(1) {
		ui.Process(hw);
		System::Delay(4);
	}
};
