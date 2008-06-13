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
#ifndef GEARS_NOTIFIER_NOTIFICATION_H__
#define GEARS_NOTIFIER_NOTIFICATION_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include "gears/base/common/serialization.h"

// Describes the information about a notification.
// Note: do not forget to increase kNotificationVersion if you make any change
// to this class.
class Notification : public Serializable {
 public:
  Notification()
      : version_(kNotificationVersion),
        display_at_time_ms_(0),
        display_until_time_ms_(0) {
  }

  void CopyFrom(const Notification& from) {
    version_ = from.version_;
    title_ = from.title_;
    subtitle_ = from.subtitle_;
    icon_ = from.icon_;
    service_ = from.service_;
    id_ = from.id_;
    description_ = from.description_;
    display_at_time_ms_ = from.display_at_time_ms_;
    display_until_time_ms_ = from.display_until_time_ms_;
  }

  virtual SerializableClassId GetSerializableClassId() {
    return SERIALIZABLE_DESKTOP_NOTIFICATION;
  }

  virtual bool Serialize(Serializer *out) {
    out->WriteInt(version_);
    out->WriteString(title_.c_str());
    out->WriteString(subtitle_.c_str());
    out->WriteString(icon_.c_str());
    out->WriteString(service_.c_str());
    out->WriteString(id_.c_str());
    out->WriteString(description_.c_str());
    out->WriteInt64(display_at_time_ms_);
    out->WriteInt64(display_until_time_ms_);
    return true;
  }

  virtual bool Deserialize(Deserializer *in) {
    if (!in->ReadInt(&version_) ||
        version_ != kNotificationVersion ||
        !in->ReadString(&title_) ||
        !in->ReadString(&subtitle_) ||
        !in->ReadString(&icon_) ||
        !in->ReadString(&service_) ||
        !in->ReadString(&id_) ||
        !in->ReadString(&description_) ||
        !in->ReadInt64(&display_at_time_ms_) ||
        !in->ReadInt64(&display_until_time_ms_)) {
      return false;
    }
    return true;
  }

  static Serializable *New() {
    return new Notification;
  }

  static void RegisterAsSerializable() {
    Serializable::RegisterClass(SERIALIZABLE_DESKTOP_NOTIFICATION, New);
  }

  bool Matches(const std::string16 &service,
               const std::string16 &id) const {
    return service_ == service && id_ == id;
  }

  const std::string16& title() const { return title_; }
  const std::string16& subtitle() const { return subtitle_; }
  const std::string16& icon() const { return icon_; }
  const std::string16& service() const { return service_; }
  const std::string16& id() const { return id_; }
  const std::string16& description() const { return description_; }
  int64 display_at_time_ms() const { return display_at_time_ms_; }
  int64 display_until_time_ms() const { return display_until_time_ms_; }

  void set_title(const std::string16& title) { title_ = title; }
  void set_subtitle(const std::string16& subtitle) { subtitle_ = subtitle; }
  void set_icon(const std::string16& icon) { icon_ = icon; }
  void set_service(const std::string16& service) { service_ = service; }
  void set_id(const std::string16& id) { id_ = id; }
  void set_description(const std::string16& description) {
    description_ = description;
  }
  void set_display_at_time_ms(int64 display_at_time_ms) {
    display_at_time_ms_ = display_at_time_ms;
  }
  void set_display_until_time_ms(int64 display_until_time_ms) {
    display_until_time_ms_ = display_until_time_ms;
  }

 private:
  static const int kNotificationVersion = 3;

  // NOTE: Increase the kNotificationVersion every time the serialization is
  // going to produce different result. This most likely includes any change
  // to data members below.
  int version_;
  std::string16 title_;
  std::string16 subtitle_;
  std::string16 icon_;
  std::string16 id_;
  std::string16 service_;
  std::string16 description_;
  int64 display_at_time_ms_;
  int64 display_until_time_ms_;
  // NOTE: Increase the kNotificationVersion every time the serialization is
  // going to produce different result. This most likely includes any change
  // to data members above.

  DISALLOW_EVIL_CONSTRUCTORS(Notification);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_NOTIFICATION_H__
