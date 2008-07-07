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

#include "gears/geolocation/position_table.h"

// Macros for use in SQL statements.
#define POSITION L"Position"
#define NAME L"Name"

// Database schema.
static const char *kCreateTableVersion1Statement =
    "CREATE TABLE Position ("
    " Name TEXT PRIMARY KEY, "
    " Latitude DOUBLE NOT NULL, "
    " Longitude DOUBLE NOT NULL, "
    " Altitude INTEGER NOT NULL, "
    " HorizontalAccuracy INTEGER NOT NULL, "
    " VerticalAccuracy INTEGER NOT NULL, "
    " Timestamp INT64 NOT NULL, "
    " Error TEXT NOT NULL, "
    " StreetNumber TEXT NOT NULL, "
    " Street TEXT NOT NULL, "
    " Premises TEXT NOT NULL, "
    " City TEXT NOT NULL, "
    " County TEXT NOT NULL, "
    " Region TEXT NOT NULL, "
    " Country TEXT NOT NULL, "
    " CountryCode TEXT NOT NULL, "
    " PostalCode TEXT NOT NULL "
    ")";

PositionTable::PositionTable(SQLDatabase *db) : db_(db) {
}

bool PositionTable::Create() {
  return CreateVersion1();
}

bool PositionTable::SetPosition(const std::string16 &name,
                                const Position &position) {
// Local helper macro.
#define LOG_BIND_ERROR(name) \
    LOG(("PositionTable::SetPosition unable to bind " name ": %d\n", \
    db_->GetErrorCode(), ".\n"));

  SQLTransaction transaction(db_, "PositionTable::SetPosition");
  if (!transaction.Begin()) {
    return false;
  }

  const char16 *sql = STRING16(L"REPLACE INTO " POSITION L" "
                               L"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, "
                               L"?, ?, ?, ?, ?)");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("PositionTable::SetPosition unable to prepare: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, name.c_str())) {
    LOG_BIND_ERROR("name");
    return false;
  }
  if (SQLITE_OK != statement.bind_double(1, position.latitude)) {
    LOG_BIND_ERROR("latitude");
    return false;
  }
  if (SQLITE_OK != statement.bind_double(2, position.longitude)) {
    LOG_BIND_ERROR("longitude");
    return false;
  }
  if (SQLITE_OK != statement.bind_int(3, position.altitude)) {
    LOG_BIND_ERROR("altitude");
    return false;
  }
  if (SQLITE_OK != statement.bind_int(4, position.horizontal_accuracy)) {
    LOG_BIND_ERROR("horizontal accuracy");
    return false;
  }
  if (SQLITE_OK != statement.bind_int(5, position.vertical_accuracy)) {
    LOG_BIND_ERROR("vertical accuracy");
    return false;
  }
  if (SQLITE_OK != statement.bind_int64(6, position.timestamp)) {
    LOG_BIND_ERROR("timestamp");
    return false;
  }
  if (SQLITE_OK != statement.bind_text16(7, position.error.c_str())) {
    LOG_BIND_ERROR("error");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(8, position.address.street_number.c_str())) {
    LOG_BIND_ERROR("street number");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(9, position.address.street.c_str())) {
    LOG_BIND_ERROR("street");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(10, position.address.premises.c_str())) {
    LOG_BIND_ERROR("premises");
    return false;
  }
  if (SQLITE_OK != statement.bind_text16(11, position.address.city.c_str())) {
    LOG_BIND_ERROR("city");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(12, position.address.county.c_str())) {
    LOG_BIND_ERROR("county");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(13, position.address.region.c_str())) {
    LOG_BIND_ERROR("region");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(14, position.address.country.c_str())) {
    LOG_BIND_ERROR("country");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(15, position.address.country_code.c_str())) {
    LOG_BIND_ERROR("country code");
    return false;
  }
  if (SQLITE_OK !=
      statement.bind_text16(16, position.address.postal_code.c_str())) {
    LOG_BIND_ERROR("postal code");
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("PositionTable::SetPosition unable to step: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  return transaction.Commit();
}

bool PositionTable::GetPosition(const std::string16 &name,
                                Position *position) {
  assert(position);

  const char16 *sql = STRING16(L"SELECT * "
                               L"FROM " POSITION L" "
                               L"WHERE " NAME L" = ? ");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("PositionTable::GetPosition unable to prepare: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, name.c_str())) {
    LOG(("PositionTable::GetPosition unable to bind name: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  int rc = statement.step();
  if (SQLITE_DONE == rc) {
    return false;
  } else if (SQLITE_ROW != rc) {
    LOG(("PositionTable::GetPosition results error: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  // Skip Name at index 0.
  position->latitude              = statement.column_double(1);
  position->longitude             = statement.column_double(2);
  position->altitude              = statement.column_int(3);
  position->horizontal_accuracy   = statement.column_int(4);
  position->vertical_accuracy     = statement.column_int(5);
  position->timestamp             = statement.column_int64(6);
  position->error                 = statement.column_text16_safe(7);
  position->address.street_number = statement.column_text16_safe(8);
  position->address.street        = statement.column_text16_safe(9);
  position->address.premises      = statement.column_text16_safe(10);
  position->address.city          = statement.column_text16_safe(11);
  position->address.county        = statement.column_text16_safe(12);
  position->address.region        = statement.column_text16_safe(13);
  position->address.country       = statement.column_text16_safe(14);
  position->address.country_code  = statement.column_text16_safe(15);
  position->address.postal_code   = statement.column_text16_safe(16);

  return true;
}

bool PositionTable::DeletePosition(const std::string16 &name) {
  const char16 *sql = STRING16(L"DELETE FROM " POSITION L" "
                               L"WHERE " NAME L" = ?");

  SQLStatement statement;
  if (SQLITE_OK != statement.prepare16(db_, sql)) {
    LOG(("PositionTable::DeletePosition unable to prepare: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  if (SQLITE_OK != statement.bind_text16(0, name.c_str())) {
    LOG(("PositionTable::DeletePosition unable to bind name: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  if (SQLITE_DONE != statement.step()) {
    LOG(("PositionTable::DeletePosition unable to step: %d\n",
         db_->GetErrorCode(), ".\n"));
    return false;
  }

  return true;
}

bool PositionTable::CreateVersion1() {
  SQLTransaction transaction(db_, "PositionTable::CreateVersion1");
  if (!transaction.Begin()) {
    return false;
  }

  if (SQLITE_OK != db_->Execute(kCreateTableVersion1Statement)) {
    LOG(("PositionTable::CreateVersion1 create Position "
         "unable to execute: %d", db_->GetErrorCode(), ".\n"));
    return false;
  }

  return transaction.Commit();
}
