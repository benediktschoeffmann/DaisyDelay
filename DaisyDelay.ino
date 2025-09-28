#define KNOB_A A0
#define KNOB_B A1
#define KNOB_C A2
#define KNOB_D A3
#define KNOB_E A4
#define KNOB_F A5


#define SWITCH_MONO   D1

#include "DaisyDuino.h"

DaisyHardware hw;

size_t num_channels;
int knobA, knobB, knobC, knobD;
bool isMono = true;

// two DelayLine of 24000 floats.
DelayLine<float, 24000> del_left, del_right;
AnalogControl ctrl_fb_l, ctrl_fb_r;
AnalogControl ctrl_mix_l, ctrl_mix_r;
AnalogControl crtl_del_l, ctrl_del_r;

float ffbl, ffbr;
float fmixl, fmixr;
float fdell, fder;


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
    del_left.Write((wet_left * ffbl) + dry_left);
    del_right.Write((wet_right * ffbr) + dry_right);

    // Mix Dry and Wet and send to I/O
    out[0][i] = wet_left * fmixl + (dry_left * (1-fmixl));
    out[1][i] = wet_right * fmixr + (dry_right * (1 - fmixl));
  }
}

void setup() {

  float sampleRate;
  // Initialize for Daisy pod at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  num_channels = hw.num_channels;

  sampleRate = DAISY.AudioSampleRate();

  // Init Pins for Switch
  pinMode(SWITCH_MONO, INPUT_PULLUP);

  // Init Delay Lines
  del_left.Init();
  del_right.Init();

  // Init Analog Ctrl
  ctrl_fb_l.Init(KNOB_A, sampleRate);
  ctrl_mix_l.Init(KNOB_B, sampleRate);
  ctrl_del_l.Init(KNOB_C, sampleRate);
  ctrl_fb_r.Init(KNOB_D, sampleRate);
  ctrl_mix_r.Init(KNOB_E, sampleRate);
  ctrl_del_r.Init(KNOB_F, sampleRate);
  
  // Set Delay Time
  del_left.SetDelay(8000.f * 0.5f);
  del_right.SetDelay(8000.f * 0.5f);

  // init values;
  ffbl = ffbr = 0.1f;
  fmixl = fmixr = 0.5f;
  fdell = fdelr = 0.5f;
  isMono = false;

  // Start Audio
  DAISY.begin(MyCallback);
}

void loop() {
  isMono = digitalRead(SWITCH_MONO);

  ffbl = ctrl_fb_l.Process();
  ffbr = ctrl_fb_r.Process();
  fdell = ctrl_del_l.Process();
  fdelr = ctrl_del_r.Process();
  fmixl = ctrl_mix_l.Process();
  fmixr = ctrl_mix_r.Process();

  del_left.SetDelay(constrain(fdell, 0.f, 1.f) * 24000);
  del_right.SetDelay(constrain(fdelr, 0.f, 1.f) * 24000);
}


