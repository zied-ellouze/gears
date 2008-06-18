// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// GearsAudioRecorder - provides audio recording abilities.

#ifndef GEARS_MEDIA_AUDIO_RECORDER_H__
#define GEARS_MEDIA_AUDIO_RECORDER_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/string16.h"
#include "gears/media/audio_recorder_constants.h"

class GearsAudioRecorder
    : public ModuleImplBaseClassVirtual {
 public:
  GearsAudioRecorder();
  ~GearsAudioRecorder();

  // ---- ERROR STATE ----
  // readonly attribute AudioRecorderError error;
  void GetError(JsCallContext *context);

  // ---- RECORDING STATE ----
  // readonly attribute boolean recording;
  void GetRecording(JsCallContext *context);

  // readonly attribute boolean paused;
  void GetPaused(JsCallContext *context);

  // readonly attribute int activityLevel;
  void GetActivityLevel(JsCallContext *contex);

  // readonly attribute float duration;
  void GetDuration(JsCallContext *context);

  // const unsinged short MONO = 1;
  DEFINE_CONSTANT(MONO, int, JSPARAM_INT,
    AudioRecorderConstants::CHANNEL_TYPE_MONO);

  // const unsinged short STEREO = 2;
  DEFINE_CONSTANT(STEREO, int, JSPARAM_INT,
    AudioRecorderConstants::CHANNEL_TYPE_STEREO);

  // attribute short channelType;
  void GetChannelType(JsCallContext *context);
  void SetChannelType(JsCallContext *context);

  // attribute int sampleRate;
  void GetSampleRate(JsCallContext *context);
  void SetSampleRate(JsCallContext *context);

  // attribute int bitsPerSample;
  void GetBitsPerSample(JsCallContext *context);
  void SetBitsPerSample(JsCallContext *context);

  // attribute string format;
  void GetFormat(JsCallContext *context);
  void SetFormat(JsCallContext *context);

  // ---- METHODS ----
  void Record(JsCallContext *context);
  void Pause(JsCallContext *context);
  void UnPause(JsCallContext *context);
  void Stop(JsCallContext *context);

  // ---- CONTROLS ----
  //  attribute float volume;
  void GetVolume(JsCallContext *context);
  void SetVolume(JsCallContext *context);

  //  attribute boolean muted;
  void GetMuted(JsCallContext *context);
  void SetMuted(JsCallContext *context);

  // attribute int silenceLevel;
  void GetSilenceLevel(JsCallContext *context);
  void SetSilenceLevel(JsCallContext *context);

  // ---- CUE RANGES ----
  // void addCueRange(in DOMString className, in float start, in float end,
  //   in boolean pauseOnExit,
  //   in VoidCallback enterCallback, in VoidCallback exitCallback);
  void AddCueRange(JsCallContext *context);

  // void removeCueRanges(in DOMString className);
  void RemoveCueRanges(JsCallContext *context);

  // ---- BLOB ----
  void GetBlob(JsCallContext *context);

  // ---- EVENTS ----
  // record, progress, error, pause, unpause, volumechange, ended
  void GetEventRecord(JsCallContext *context);
  void SetEventRecord(JsCallContext *context);
  void GetEventProgress(JsCallContext *context);
  void SetEventProgress(JsCallContext *context);
  void GetEventError(JsCallContext *context);
  void SetEventError(JsCallContext *context);
  void GetEventPause(JsCallContext *context);
  void SetEventPause(JsCallContext *context);
  void GetEventUnPause(JsCallContext *context);
  void SetEventUnPause(JsCallContext *context);
  void GetEventVolumeChange(JsCallContext *context);
  void SetEventVolumeChange(JsCallContext *context);
  void GetEventEnded(JsCallContext *context);
  void SetEventEnded(JsCallContext *context);

 private:
  int last_error_;

  bool recording_;
  bool paused_;
  int activity_level_;
  double duration_;

  int channel_type_;
  int sample_rate_;
  int bits_per_sample_;
  std::string16 format_;

  double volume_;
  bool muted_;
  int silence_level_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsAudioRecorder);
};
#endif  // GEARS_MEDIA_AUDIO_RECORDER_H__
