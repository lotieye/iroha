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

#include <string>
#include <thread>
#include <unordered_map>
#include <util/exception.hpp>
#include <util/logger.hpp>
#include <vm/vm.hpp>
#include "java_context.hpp"

namespace vm {

enum class VMState { NotInitialized, Running, Finished };
struct ChaincodeName {
  std::string package, code;
  ChaincodeName(const std::string &package, const std::string &code)
      : package(package), code(code) {}
  struct Hash {
    std::size_t operator() const { return std::hash(pakcage + code); }
  };
};

using ContextMap =
    std::unordered_map<ChaincodeName, JavaContext, ChaincodeName::Hash>;

static VMState vmState;
static ContextMap contextMap;

namespace detail {

const std::vector<std::string> javaArgs() {
  using Config = config::IrohaConfigManager::getInstance();
  return {"-Djava.class.path=" + IrohaHome + "/" +
              Config.getJavaClassPath("java_tests") + ":" + IrohaHome + "/" +
              Config.getJavaClassPathLocal("smart_contract/java_tests"),

          "-Djava.library.path=" + IrohaHome + "/" +
              Config.getJavaLibraryPath("lib") + ":" + IrohaHome + "/" +
              Config.getJavaLibraryPathLocal("build/lib"),

          "-Djava.security.policy=" + IrohaHome + "/" +
              Config.getJavaPolicyPath("jvm/java.policy.txt"),

          "-Djava.security.manager"};
}
}

/**
 * Initialize JVM.
 * - This method is called only once in Iroha.
 */
void initialize() {

  IROHA_ASSERT_TRUE(vmState == VMState::NotInitialized);
  vmState = VMState::Running;

  logger::debug("vm") << "start JVM";

  const auto IrohaHome = []() {
    const auto p = getenv("IROHA_HOME");
    IROHA_ASSERT_TRUE(p && "[FATAL] Set environment $IROHA_HOME")
    return std::string(p);
  }();

  const auto packageName = "instances." + packageNameUnderInstances;

  // paths are hard coding here...
  std::vector<std::string> java_args =

      const int OptionSize = java_args.size();

  JavaVMOption options[OptionSize];
  for (int i = 0; i < OptionSize; i++) {
    options[i].optionString = const_cast<char *>(java_args[i].c_str());
  }

  {
    for (int i = 0; i < OptionSize; i++) {
      std::cout << options[i].optionString << " ";
    }
    std::cout << packageName + "." + contractName << std::endl;
  }

  JavaVMInitArgs vm_args;
  vm_args.version = JNI_VERSION_1_8;
  vm_args.options = options;
  vm_args.nOptions = OptionSize;
  vm_args.ignoreUnrecognized = JNI_FALSE;

  JNIEnv *env;
  JavaVM *jvm;

  int res = JNI_CreateJavaVM(&jvm, (void **)&env, &vm_args);
  if (res) {
    std::cout << "cannot run JavaVM : " << res << std::endl;
    return nullptr;
  }
}

/**
 * Destroy JVM.
 * - This method is called only once at the end of Iroha.
 */
void destroy() {
  IROHA_ASSERT_TRUE(vmState == VMState::Running);
  vmState = VMState::Finished;
  logger::debug("vm") << "finish JVM";
}

/**
 * Initialize chaincode thread.
 * - invokes new thread to run chaincode.
 */
void initChaincode(const ChaincodeName &name, const ByteArray &byteCode) {
  IROHA_ASSERT_TRUE(contextMap.find(name) == contextMap.end());

  /////////////////////////////////////////////////////////////////////////////////////////////////
  auto slashPackageName = packageName;
  std::transform(slashPackageName.begin(), slashPackageName.end(),
                 slashPackageName.begin(),
                 [](const char a) { return a == '.' ? '/' : a; });

  jclass cls = env->FindClass((slashPackageName + "/" + contractName).c_str());
  if (cls == nullptr) {
    std::cout << "could not found class : " << packageName << "."
              << contractName << std::endl;
    return nullptr;
  }

  jmethodID cns = env->GetMethodID(cls, "<init>", "()V");
  if (cns == nullptr) {
    std::cout << "could not get <init> method." << std::endl;
    return nullptr;
  }

  jobject obj = env->NewObject(cls, cns);

  return std::make_unique<JavaContext>(
      env, jvm, vm_args, packageName + "." + contractName, cls, obj);
}

void finishChaincode(const ChaincodeName &name) {
  IROHA_ASSERT_TRUE(contextMap.find(name) != contextMap.end());
}

void initializeVM(const std::string &packageName,
                  const std::string &contractName) {
  const auto NameId = pack(packageName, contractName);
  if (vmSet.find(NameId) != vmSet.end()) {
    // http://bugs.java.com/bugdatabase/view_bug.do?bug_id=4479303
    // fork() or exec() needed? ref:
    // http://stackoverflow.com/questions/2259947/creating-a-jvm-from-within-a-jni-method
    logger::fatal("virtual machine with java")
        << "Currently, not supported for initializing VM twice.";
    exit(EXIT_FAILURE);
    //            vmSet.at(NameId)->jvm->DestroyJavaVM();
    //            vmSet.erase(NameId);
  }
  vmSet.emplace(NameId, jvm::initializeVM(packageName, contractName));
}

void finishVM(const std::string &packageName, const std::string &contractName) {
  const auto NameId = pack(packageName, contractName);
  if (vmSet.find(NameId) != vmSet.end()) {
    vmSet.at(NameId)->jvm->DestroyJavaVM();
    //            vmSet.erase(NameId);
  }
}

void invokeFunction(const std::string &packageName,
                    const std::string &contractName,
                    const std::string &functionName) {
  const auto NameId = pack(packageName, contractName);
  if (vmSet.find(NameId) != vmSet.end()) {
    const auto &context = vmSet.at(NameId);
    jvm::execFunction(context, functionName);
  }
}

void invokeFunction(const std::string &packageName,
                    const std::string &contractName,
                    const std::string &functionName,
                    const std::map<std::string, std::string> &params) {
  const auto NameId = pack(packageName, contractName);
  if (vmSet.find(NameId) != vmSet.end()) {
    const auto &context = vmSet.at(NameId);
    jvm::execFunction(context, functionName, params);
  }
}

void invokeFunction(
    const std::string &packageName, const std::string &contractName,
    const std::string &functionName,
    const std::map<std::string, std::string> &params1,
    const std::map<std::string, std::map<std::string, std::string>> &params2) {
  const auto NameId = pack(packageName, contractName);
  if (vmSet.find(NameId) != vmSet.end()) {
    const auto &context = vmSet.at(NameId);
    jvm::execFunction(context, functionName, params1, params2);
  }
}

void invokeFunction(const std::string &packageName,
                    const std::string &contractName,
                    const std::string &functionName,
                    const std::map<std::string, std::string> &params1,
                    const std::map<std::string, std::string> &params2) {
  const auto NameId = pack(packageName, contractName);
  if (vmSet.find(NameId) != vmSet.end()) {
    const auto &context = vmSet.at(NameId);
    jvm::execFunction(context, functionName, params1, params2);
  }
}

void invokeFunction(const std::string &packageName,
                    const std::string &contractName,
                    const std::string &functionName,
                    const std::map<std::string, std::string> &params1,
                    const std::vector<std::string> &params2) {
  const auto NameId = pack(packageName, contractName);
  if (vmSet.find(NameId) != vmSet.end()) {
    const auto &context = vmSet.at(NameId);
    jvm::execFunction(context, functionName, params1, params2);
  }
}
}
