#include <iostream>
#include <assert.h>
#include <cstring>
#include <memory>
#include <thread>

#include <grpc++/grpc++.h>

#include "data_generated.h"
#include "data.grpc.fb.h"

namespace model = CppFlatBufGrpc::Sample;

// コールバック
class ServiceImpl final : public model::DataStorage::Service {
  virtual ::grpc::Status Store(
    ::grpc::ServerContext* context,
    flatbuffers::BufferRef<model::Data> const* request,
    flatbuffers::BufferRef<model::Stat>* response
  ) override {
    fbb_.Clear();
    auto stat_offset = model::CreateStat(
      fbb_,
      fbb_.CreateString("Hello, " + request->GetRoot()->name()->str())
    );
    fbb_.Finish(stat_offset);

    *response = flatbuffers::BufferRef<model::Stat>(fbb_.GetBufferPointer(),
                                                    fbb_.GetSize());
    return grpc::Status::OK;
  }

  virtual ::grpc::Status Retrieve(
    ::grpc::ServerContext* context,
    const flatbuffers::BufferRef<model::Stat>* request,
    ::grpc::ServerWriter<flatbuffers::BufferRef<model::Data>>* writer
  ) override {
    std::cout << "TEST\n";
    return grpc::Status::CANCELLED;
  }

private:
  flatbuffers::FlatBufferBuilder fbb_;
};

grpc::Server* server_instance = nullptr;
std::mutex wait_for_server;
std::condition_variable server_instance_cv;

void RunServer() {
  const auto server_address = "0.0.0.0:50051";

  ServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort(
    server_address,
    grpc::InsecureServerCredentials()
  );
  builder.RegisterService(&service);

  // Start the serer. Lock to change the variable we're changing.
  wait_for_server.lock();
  {
    server_instance = builder.BuildAndStart().release();
  }
  wait_for_server.unlock();
  server_instance_cv.notify_one(); // What's this?

  std::cout << "Server listening on " << server_address << std::endl;
  
  // This will block the thread and serve requests.
  server_instance->Wait();
}

void mainmain() {

  // FlatBufferBuilderの宣言
  // 作成した値はFlatBufferBuilderのメンバ変数の省エネなvectorのバッファに溜め込まれていく
  flatbuffers::FlatBufferBuilder fbb_;

  // tableの子要素を生成。
  // Stringをバッファリングする。
  // Offset<...>によって、バッファリングのたびに位置情報と使用された型を記録(テンプレートパラメータ)する。
  flatbuffers::Offset<flatbuffers::String> name = fbb_.CreateString("Hello");
  int32_t value = 12345;

  // tableを生成
  // 子要素が全て生成できてから、親要素の生成に入るようにする。これを破るとNotNested()の
  // アサーションに引っかかる。
  #if 1
  // ビルダーを使う方法
  model::DataBuilder builder(fbb_);
  builder.add_name(name);
  builder.add_value(value);
  flatbuffers::Offset<model::Data> data = builder.Finish();
  #else
  // ビルダーを使わない方法
  flatbuffers::Offset<model::Data> data = model::CreateData(fbb_, name, value);
  #endif

  // データを転送できる状態にする
  model::FinishDataBuffer(fbb_, data);

  // FBB内部のバイト列を取得
  std::vector<uint8_t> buffer;
  auto ptr = reinterpret_cast<const uint8_t*>(fbb_.GetBufferPointer());
  buffer.assign(ptr, ptr + fbb_.GetSize());

  // 所有権を移譲してバイナリデータのポインタを受け取る
  flatbuffers::unique_ptr_t flatbuf = fbb_.ReleaseBufferPointer();
  //// Do not use fbb_ afterwards.

  // バイナリデータのポインタをflatbuffersのデータ構造のポインタにキャスト
  model::Data const* rdata1 = model::GetData(reinterpret_cast<const uint8_t*>(buffer.data()));
  model::Data const* rdata2 = model::GetData(flatbuf.get());

  assert(0 == std::strcmp(rdata1->name()->c_str(), rdata2->name()->c_str()));

  // 標準ライブラリのデータ構造に変換 (速度犠牲、使いやすさ向上)
  std::unique_ptr<model::DataT> stdRData1(rdata1->UnPack());
  std::unique_ptr<model::DataT> stdRData2(rdata2->UnPack());

  assert(stdRData1->name == stdRData2->name);
  assert(typeid(stdRData1->name) == typeid(std::string));

  assert(stdRData1->value == stdRData2->value);
  assert(typeid(stdRData1->value) == typeid(int));

  assert(typeid(model::Data) == typeid(std::remove_reference_t<decltype(*stdRData1.get())>::TableType));
  assert(typeid(model::DataT) == typeid(model::Data::NativeTableType));

  // 標準ライブラリのデータ構造からflatbuffersのデータ構造に変換
  stdRData1->name = "Goodbye!";

  flatbuffers::FlatBufferBuilder fbb2_;
  
  // Offset<Data>に変換
  flatbuffers::Offset<model::Data> edited = model::Data::Pack(fbb2_, stdRData1.get());

  model::FinishDataBuffer(fbb2_, edited);

  std::vector<uint8_t> buffer2;
  auto ptr2 = reinterpret_cast<const uint8_t*>(fbb2_.GetBufferPointer());
  buffer2.assign(ptr2, ptr2 + fbb2_.GetSize());
  model::Data const* rrdata = model::GetData(reinterpret_cast<const uint8_t*>(buffer2.data()));

  printf("%s\n", rrdata->name()->c_str());
  printf("%d\n", rrdata->value());

  assert(0 == strcmp("Goodbye!", rrdata->name()->c_str()));
  assert(12345 == rrdata->value());

}

int main() {
  std::thread server_thread(RunServer);

  std::unique_lock<std::mutex> lock(wait_for_server);
  while(!server_instance) server_instance_cv.wait(lock);

  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  auto stub = model::DataStorage::NewStub(channel);

  grpc::ClientContext context;

  flatbuffers::FlatBufferBuilder fbb;
  flatbuffers::Offset<model::Data> dataOffset = model::CreateData(fbb, fbb.CreateString("GRPC_FLATBUF_MAIN"), 12345);
  fbb.Finish(dataOffset);
  auto request = flatbuffers::BufferRef<model::Data>(fbb.GetBufferPointer(), fbb.GetSize());
  flatbuffers::BufferRef<model::Stat> response;

  // The actual RPC.
  auto status = stub->Store(&context, request, &response);

  if (status.ok()) {



    std::cout << "RPC response: "
              << response.GetRoot()->id() << ", "
              << response.GetRoot()->val() << ", "
              << std::endl;
  } else {
    std::cout << "RPC failed" << std::endl;
  }

  server_instance->Shutdown();

  server_thread.join();

  delete server_instance;

}