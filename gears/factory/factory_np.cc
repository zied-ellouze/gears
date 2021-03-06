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

#include "gears/factory/factory_np.h"

#include "gears/base/npapi/module_wrapper.h"
#include "gears/factory/factory_impl.h"

NPObject* CreateGearsFactoryWrapper(JsContextPtr context) {
  scoped_ptr<ModuleWrapper> factory_wrapper(static_cast<ModuleWrapper*>(
        NPN_CreateObject(context, ModuleWrapper::GetNPClass())));
  if (factory_wrapper.get()) {
    scoped_refptr<ModuleEnvironment> module_environment(
        ModuleEnvironment::CreateFromDOM(context));
    if (!module_environment) {
      return NULL;
    }
    GearsFactoryImpl *factory_impl = new GearsFactoryImpl;
    factory_impl->InitModuleEnvironment(module_environment.get());
    factory_wrapper->Init(factory_impl,
                          new Dispatcher<GearsFactoryImpl>(factory_impl));
  }

  return factory_wrapper.release();
}
