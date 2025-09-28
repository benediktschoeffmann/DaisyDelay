#define KNOB_A A0
#define KNOB_B A1
#define KNOB_C A2
#define KNOB_D A3

#define SWITCH_MONO   D1

#include "DaisyDuino.h"

DaisyHardware hw;

size_t num_channels;
int knobA, knobB, knobC, knobD;
bool isMono = true;

// two DelayLine of 24000 floats.
DelayLine<float, 22000> del_left, del_right;
AnalogControl ctrl_fb_l, ctrl_fb_r;
AnalogControl ctrl_mix_l, ctrl_mix_r;

float ffbl, ffbr;
float fmixl, fmixr;


void MyCallback(float **in, float **out, size_t size) {
  for (size_t i = 0; i < size; i++) {
    float dry_left, dry_right, wet_left, wet_right;

    // Read Dry from I/O
    dry_left = in[0][i];
    if (isMono) {
      dry_right = in[0][i];  
    } else {
      dry_right = in[1][i];
    }

    // Read Wet from Delay Lines
    wet_left = del_left.Read();
    wet_right = del_right.Read();

    // Write to Delay with some feedback
    del_left.Write((wet_left * 0.5) + dry_left);
    del_right.Write((wet_right * 0.5) + dry_right);

    // Mix Dry and Wet and send to I/O
    out[0][i] = wet_left * 0.707 + dry_left * 0.707;
    out[1][i] = wet_right * 0.707 + dry_right * 0.707;
  }
}

void setup() {

  float sampleRate;
  // Initialize for Daisy pod at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  num_channels = hw.num_channels;

  sampleRate = DAISY.AudioSampleRate();

  // Init Pins for Switch
  // gpio_is_mono.Init(SWITCH_MONO, GPIO::Mode::INPUT, GPIO::Pull::PULLUP);
  pinMode(SWITCH_MONO, INPUT_PULLUP);

  // Init Delay Lines
  del_left.Init();
  del_right.Init();

  // Init Analog Ctrl
  ctrl_fb_l.Init(KNOB_A, sampleRate);
  ctrl_fb_r.Init(KNOB_B, sampleRate);
  ctrl_mix_l.Init(KNOB_C, sampleRate);
  ctrl_mix_r.Init(KNOB_D, sampleRate);

  // Set Delay Times in Samples
  del_left.SetDelay(2000.0f);
  del_right.SetDelay(2000.0f);

  // Start Audio
  DAISY.begin(MyCallback);
}

void loop() {
  isMono = digitalRead(SWITCH_MONO);

  ffbl = ctrl_fb_l.Process();
  ffbr = ctrl_fb_r.Process();
  fmixl = ctrl_mix_l.Process();
  fmixr = ctrl_mix_r.Process();
}


