/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

#define  TEST_ETCD 1
#ifdef TEST_ETCD
#include "lock.grpc.pb.h"
#include "rpc.grpc.pb.h"
#endif 


using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

class EtcdTest
{
public:
  EtcdTest(std::shared_ptr<Channel> channel)
    : stub_(::etcdserverpb::KV::NewStub(channel)) {}

  std::string PutTests()
  {
      ::etcdserverpb::TxnRequest request;
      //auto sPtrCom = request.add_compare();
      //sPtrCom->set_result(::etcdserverpb::Compare_CompareResult::Compare_CompareResult_EQUAL);
      //sPtrCom->set_target(::etcdserverpb::Compare_CompareTarget::Compare_CompareTarget_VALUE);
      //sPtrCom->set_key("tystest1");
      //sPtrCom->set_value("1");

      //auto sPtrCom1 = request.add_compare();
      //sPtrCom1->set_result(::etcdserverpb::Compare_CompareResult::Compare_CompareResult_EQUAL);
      //sPtrCom1->set_target(::etcdserverpb::Compare_CompareTarget::Compare_CompareTarget_VALUE);
      //sPtrCom1->set_key("tystest2");
      //sPtrCom1->set_value("2");

      auto sPtrSu0 = request.add_success();
      sPtrSu0->mutable_request_put()->set_key("tystest3");
      sPtrSu0->mutable_request_put()->set_value("3_1");


      auto sPtrSu1 = request.add_success();
      sPtrSu1->mutable_request_put()->set_key("tystest4");
      sPtrSu1->mutable_request_put()->set_value("4_1");


       auto sPtrOP =  request.add_failure();
       sPtrOP->mutable_request_put()->set_key("tystest1");
       sPtrOP->mutable_request_put()->set_value("1");

       auto sPtrOP1 = request.add_failure();
       sPtrOP1->mutable_request_put()->set_key("tystest2");
       sPtrOP1->mutable_request_put()->set_value("2");

       ClientContext context;
       ::etcdserverpb::TxnResponse response;
       Status status = stub_->Txn(&context,request,&response);


       // Act upon its status.
       if (status.ok()) {
         return response.DebugString();
       }
       else {
         std::cout << status.error_code() << ": " << status.error_message()
           << std::endl;
         return "RPC failed";
       }
  }

  std::string RangeEtcd()
  {
    etcdserverpb::RangeRequest request;
    request.set_key("tystest1");
    request.set_range_end("tystest5");
   // request.set_count_only(true);

    ClientContext context;

    ::etcdserverpb::RangeResponse response;
    Status status = stub_->Range(&context, request, &response);

      if (status.ok()) {

        std::string sRe = response.DebugString();
       auto sKvres = response.kvs_size();
        return sRe;
      }
      else {
        std::cout << status.error_code() << ": " << status.error_message()
          << std::endl;
        return "RPC failed";
      }
  }
private:
  std::unique_ptr<::etcdserverpb::KV::Stub> stub_;
};

#ifdef HELLO_WORLD
int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  GreeterClient greeter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  std::string user("world");
  std::string reply = greeter.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}
#endif 

#ifdef TEST_ETCD

int main()
{
  EtcdTest etcdtest(grpc::CreateChannel("10.0.14.80:2379", grpc::InsecureChannelCredentials()));

 // auto reply = etcdtest.PutTests();

  auto sRes = etcdtest.RangeEtcd();


  return 0;
}
#endif
