// Copyright 2007, Google Inc.
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

#include <assert.h>
#include "gears/base/common/common.h"
#include "gears/base/common/timer.h"

//------------------------------------------------------------------------------
// Start
//------------------------------------------------------------------------------
void Timer::Start() {
  MutexLock lock(&mutex_);

  if (nested_count_ == 0) {
    start_ = static_cast<int>(GetCurrentTimeMillis());
  }

  nested_count_++;
}

//------------------------------------------------------------------------------
// Stop
//------------------------------------------------------------------------------
void Timer::Stop() {
  MutexLock lock(&mutex_);

  // You shouldn't call stop() before ever calling start; that would be silly.
  assert(nested_count_ > 0);

  nested_count_--;
  if (nested_count_ == 0) {
    total_ += (static_cast<int>(GetCurrentTimeMillis()) - start_);
  }
}

//------------------------------------------------------------------------------
// GetElapsed
//------------------------------------------------------------------------------
int Timer::GetElapsed() {
  return total_;
}


//------------------------------------------------------------------------------
// ScopedTimer constructor
//------------------------------------------------------------------------------
ScopedTimer::ScopedTimer(Timer *t) {
  assert(t);
  t_ = t;
  t_->Start();
}

//------------------------------------------------------------------------------
// Timing destructor
//------------------------------------------------------------------------------
ScopedTimer::~ScopedTimer() {
  assert(t_);
  t_->Stop();
}
