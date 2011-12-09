// Copyright 2011, Nadav Samet.
// All rights reserved.
//
// Author: thesamet@gmail.com <Nadav Samet>

#include <iostream>
#include <glog/logging.h>
#include <google/gflags.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/text_format.h>
#include "zrpc/event_manager.h"
#include "zrpc/rpc_channel.h"
#include "zrpc/service.h"
#include "zrpc/rpc.h"
#include <zmq.hpp>


static bool ValidateNotEmpty(const char* flagname, const std::string& value) {
  if (value.empty()) {
    std::cerr << "Value for --" << flagname << " is required." << std::endl;
    return false;
  }
  return true;
}
DEFINE_string(proto, "", "Service proto to use.");
static const bool proto_dummy = ::google::RegisterFlagValidator(
    &FLAGS_proto, &ValidateNotEmpty);

DEFINE_string(proto_path, "", "list of directories to search.");

namespace zrpc {
class ErrorCollector :
    public ::google::protobuf::compiler::MultiFileErrorCollector {
  void AddError(
      const std::string& filename, int line, int column,
      const std::string& message) {
    std::cerr << filename << ":" << line << ":" << message << std::endl;
  }
};

void RunCall(const std::string& endpoint,
             const std::string& method,
             const std::string& payload) {
  ::google::protobuf::compiler::DiskSourceTree disk_source_tree;
  ErrorCollector error_collector;
  disk_source_tree.MapPath(FLAGS_proto_path, FLAGS_proto_path);
  ::google::protobuf::compiler::Importer imp(
      &disk_source_tree, &error_collector);

  const ::google::protobuf::FileDescriptor* file_desc = imp.Import(
      FLAGS_proto);
  if (file_desc == NULL) {
    return;
  }
  if (method.find('.') == method.npos) {
    std::cerr << "<service.method> must contain a dot: '" << method << "'"
              << std::endl;
    return;
  }
  std::string service_name(method, 0, method.find_last_of('.'));
  std::string method_name(method, method.find_last_of('.') + 1);
  const ::google::protobuf::ServiceDescriptor* service_desc =
      file_desc->FindServiceByName(service_name);
  if (service_desc == NULL) {
    std::cerr << "Could not find service '" << service_name
              << "' in proto definition.";
    return;
  }
  const ::google::protobuf::MethodDescriptor* method_desc =
      service_desc->FindMethodByName(method_name);
  if (method_desc == NULL) {
    std::cerr << "Could not find method '" << method_name
              << "' in proto definition (but service was found).";
    return;
  }

  ::google::protobuf::DynamicMessageFactory factory;
  ::google::protobuf::Message *request = factory.GetPrototype(
      method_desc->input_type())->New();
  CHECK_NOTNULL(request);
  if (!::google::protobuf::TextFormat::ParseFromString(payload, request)) {
    std::cerr << "Could not parse the given ASCII message." << std::endl;
    return;
  }

  zmq::context_t context(1);
  zrpc::EventManager em(&context, 1);
  scoped_ptr<Connection> connection(Connection::CreateConnection(&em,
          endpoint));
  scoped_ptr<RpcChannel> channel(connection->MakeChannel());
  RPC rpc;
  ::google::protobuf::Message *reply = factory.GetPrototype(
      method_desc->output_type())->New();
  channel->CallMethod(method_desc, &rpc, request, reply, NULL);
  rpc.Wait();

  if (rpc.GetStatus() != GenericRPCResponse::OK) {
    std::cerr << "Status: " << rpc.GetStatus() << std::endl;
    std::cerr << "Error " << rpc.GetApplicationError() << ": "
        << rpc.GetErrorMessage() << std::endl;
  } else {
    std::string out;
    ::google::protobuf::TextFormat::PrintToString(*reply, &out);
    std::cerr << out << std::endl;
  }
  delete request;
  delete reply;
}

int Run(int argc, char *argv[]) {
  if (argc == 1) {
    std::cerr << "Expecting at least one command" << std::endl;
    return -1;
  }
  std::string command(argv[1]);
  if (command != "call") {
    std::cerr << "Only the call command is supported" << std::endl;
    return -1;
  } else {
    if (argc != 5) {
      std::cerr << "call <endpoint> <service.method> <payload>" << std::endl;
      return -1;
    }
    std::string endpoint(argv[2]);
    std::string method(argv[3]);
    std::string payload(argv[4]);
    RunCall(endpoint, method, payload);
  }
  return 0;
}
}  // namespace zrpc

int main(int argc, char *argv[]) {
  ::google::InitGoogleLogging(argv[0]);
  ::google::SetUsageMessage("Sends RPCs");
  ::google::ParseCommandLineFlags(&argc, &argv, true);
  ::google::InstallFailureSignalHandler();

  int retval = zrpc::Run(argc, argv);
  if (retval == -1) {
    ::google::ShowUsageWithFlagsRestrict(argv[0], "zrpc");
  }

  ::google::protobuf::ShutdownProtobufLibrary();
  ::google::ShutdownGoogleLogging();
  return retval;
}
