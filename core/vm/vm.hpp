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

#ifndef CORE_VM_HPP
#define CORE_VM_HPP

#include <map>
#include <string>
#include <vector>

namespace vm {

using byte_t = uint8_t;
using ByteArray = std::vector<byte_t>;

void initialize(const std::string &packageName, const std::string &codeName,
                const ByteArray &byteCode);

void destroy(const std::string &packageName, const std::string &codeName);

template <class InputT, class OutputT>
void invoke(const InputT &, OutputT &);
}

#endif