#include "daisy_pod.h"
#include "string/string.h"
#include "ui/pod_ui.h"
#include "log.h"

using namespace daisy;
using namespace synthux;

DaisyPod hw;
String engine;
PodUI ui(hw, engine);


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
	HW::hw().setHW(&hw.seed);
	HW::hw().startLog();
	#endif

	engine.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
	ui.Init();

	hw.StartAdc();
	hw.StartAudio(AudioCallback);

	while(1) {
		ui.Process();
		System::Delay(4);
	}
};
