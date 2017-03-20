/*
 * Copyright 2016 Soramitsu Co., Ltd.
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
 */

package api;

import java.util.List;
import java.util.ArrayList;

/////////////////////////////////////
public class Message {
  public Array<byte> data;
}

/////////////////////////////////////
enum ProgrammingLanguage {
  Java8, Python3
}

public class Chaincode {
  public String codeName;
  public String domainName;
  public String ledgerName;
  public ProgrammingLanguage language;
  public Array<byte> code;
}

/////////////////////////////////////
public class KeyValueObject {
  public String key;
  public Array<byte> value;
}

/////////////////////////////////////
public class UserPermission {}

/////////////////////////////////////
public class UserPermissionRoot extends UserPermission {
  public Boolean domainAdd;
  public Boolean domainRemove;
  public Boolean userAdd;
  public Boolean userRemove;
  public Boolean userGivePermission;
}

/////////////////////////////////////
public class UserPermissionDomain extends UserPermission {
  public String domainName;
  public String ledgerName;

  public Boolean userGivePermission;
  public Boolean userAdd;
  public Boolean userRemove;
}

/////////////////////////////////////
public class UserPermissionAsset extends UserPermission {
  public String assetName;
  public String domainName;
  public String ledgerName;

  public Boolean transfer;
  public Boolean add;
  public Boolean remove;
  public Boolean create;
}

/////////////////////////////////////
public class PeerPermission {
  public Boolean joinNetwork;
  public Boolean joinValudation;
}

public class Peer {
  public PublicKey publicKey;
  public String ip;
  public Double trust;
  public Boolean active;
  public List<PeerPermission> permissions;
}

/////////////////////////////////////
public class Signature {
  public PublicKey publicKey;
  public Array<byte> signature;
  public Long timestamp;
}
