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

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include "gears/notifier/notifier_proxy.h"

#include <utility>
#include <vector>

#include "gears/base/common/common.h"
#include "gears/base/common/event.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/thread.h"
#include "gears/notifier/notification.h"
#include "gears/notifier/notifier_process.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class NotifierProxyThread : public Thread {
 public:
   NotifierProxyThread(int message_type, GearsNotification *notification)
      : process_started_(false) {
    assert(message_type);
    assert(notification);

    notifications_.push_back(
        std::pair<int, GearsNotification*>(message_type, notification));
    Start();
  }

  virtual ~NotifierProxyThread() {
    stop_event_.Signal();
    Join();

    for (size_t i = 0; i < notifications_.size(); ++i) {
      delete notifications_[i].second;
    }
    notifications_.clear();
  }

  bool PostNotification(int message_type, GearsNotification *notification) {
    assert(message_type);
    assert(notification);

    MutexLock locker(&lock_);

    if (process_started_) {
      return false;
    }

    notifications_.push_back(
        std::pair<int, GearsNotification*>(message_type, notification));
    return true;
  }

 private:
  virtual void Run() {
    if (!NotifierProcess::StartProcess(&stop_event_)) {
      return;
    }
    uint32 process_id = NotifierProcess::FindProcess();
    if (!process_id) {
      return;
    }

    MutexLock locker(&lock_);

    process_started_ = true;

    IpcMessageQueue *ipc_message_queue = IpcMessageQueue::GetSystemQueue();
    assert(ipc_message_queue);
    if (ipc_message_queue) {
      std::vector<std::pair<int, GearsNotification*> >::iterator iter = 
          notifications_.begin();
      while (!stop_event_.WaitWithTimeout(1) && iter != notifications_.end()) {
        ipc_message_queue->Send(static_cast<IpcProcessId>(process_id),
                                iter->first,
                                iter->second);
        iter = notifications_.erase(iter);
      }
    }
  }

  bool process_started_;
  Mutex lock_;
  Event stop_event_;
  std::vector<std::pair<int, GearsNotification*> > notifications_;

  DISALLOW_EVIL_CONSTRUCTORS(NotifierProxyThread);
};

NotifierProxy::NotifierProxy() {
}

NotifierProxy::~NotifierProxy() {
}

void NotifierProxy::PostNotification(int message_type,
                                     GearsNotification *notification) {
  assert(message_type);
  assert(notification);

  if (thread_.get()) {
    if (thread_->IsRunning()) {
      if (thread_->PostNotification(message_type, notification)) {
        return;
      }
    } else {
      thread_.reset();
    }
  }

  uint32 process_id = NotifierProcess::FindProcess();
  if (process_id) {
    IpcMessageQueue *ipc_message_queue = IpcMessageQueue::GetSystemQueue();
    assert(ipc_message_queue);
    if (ipc_message_queue) {
      ipc_message_queue->Send(static_cast<IpcProcessId>(process_id),
                              message_type,
                              notification);
    }
    return;
  }

  thread_.reset(new NotifierProxyThread(message_type, notification));
  assert(thread_.get());
}

#endif  // OFFICIAL_BUILD