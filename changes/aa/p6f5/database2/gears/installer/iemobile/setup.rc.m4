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

#include "afxres.h"
#include "installer/iemobile/resource.h"

//-----------------------------------------------------------------------------
// Strings for the setup dialog
// TODO(andreip): Localization.
//-----------------------------------------------------------------------------
LANGUAGE LANG_ENGLISH, SUBLANG_ENGLISH_US

STRINGTABLE
BEGIN
    IDS_RESTART_QUESTION "Internet Explorer must be restarted for Gears to be installed.\n\nWould you like to restart Internet Explorer and continue the installation?"
    IDS_RESTART_DIALOG_TITLE "Google Gears - Installing"
    IDS_STOP_FAILURE_MESSAGE "Failed to stop Internet Explorer. Please stop it manually and restart this installation."
    IDS_START_FAILURE_MESSAGE "Failed to start Internet Explorer. Please start it manually."
END
