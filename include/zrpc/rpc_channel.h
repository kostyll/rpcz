// Copyright 2011 Google Inc. All Rights Reserved.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: nadavs@google.com <Nadav Samet>

#ifndef ZRPC_RPC_CHANNEL_H
#define ZRPC_RPC_CHANNEL_H

#include <string>
#include <set>

#include "google/protobuf/stubs/common.h"
#include "zrpc/macros.h"

namespace google {
namespace protobuf {
class Message;
class MethodDescriptor;
}  // namespace protobuf
}  // namespace google

namespace zrpc {
class Closure;
class RPC;

class RpcChannel {
 public:
  virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          RPC* rpc,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          Closure* done) = 0;

  // DO NOT USE: this method exists only for language bindings and may be
  // removed.
  virtual void CallMethod0(const std::string& service_name,
                           const std::string& method_name,
                           RPC* rpc,
                           const std::string& request,
                           std::string* response,
                           Closure* done) = 0;

  virtual ~RpcChannel() {};
};
}  // namespace
#endif