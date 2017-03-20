/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CORE_JAVA_CONTEXT_HPP
#define CORE_JAVA_CONTEXT_HPP

#include <jni.h>
#include <string>
#include <thread>

namespace vm {

/**
 * JavaContext
 * - wrapper of chaincode context per thread
 */
struct JavaContext {
  std::thread th;
  JNIEnv* env;
  JavaVM* jvm;
  jclass jClass;
  jobject jObject;
  std::string name;
  JavaVMInitArgs vmArgs;

  JavaContext(std::thread&& th, JNIEnv* aEnv, JavaVM* aJvm,
              JavaVMInitArgs&& aArgs, std::string&& aName, jclass&& cls,
              jobject&& obj)
      : th(th),
        env(aEnv),
        jvm(aJvm),
        jClass(std::move(cls)),
        jObject(std::move(obj)),
        name(std::move(aName)),
        vmArgs(std::move(aArgs)) {}
};
}  // namespace vm

#endif  // CORE_JAVA_CONTEXT_HPP
