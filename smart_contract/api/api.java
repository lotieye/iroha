/*
Copyright 2016 Soramitsu Co., Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package api;

//union Object { Currency, KeyValueListAsset, Domain, Account, Peer, Message }
//enum Command : byte { Add = 1, Transfer, Update, Remove, Batch, Contract, ChaincodeBatch }

class Domain {
  public String name;
  public ArrayList<Permission> permissions;
  public String ledgerId;
}

class Account {
  public String publicKey;
  public String alias;
  public ArrayList<Domain> domains;
}

class Trust {
  public Long value;
  public Boolean isOk;
}

class Peer {
  public String publicKey;
  public String address;
  public Trust  trust;
}

class Signature {
  public String publicKey;
  public String signature;
}

class Permission {
  public Boolean readable;
  public Boolean writable;
  public Boolean isRoot;
  public String  publicKey;
}

// Objects
class ValueObject {
  public String   valueString;
  public Integer  value;
  public Integer  precision;
  public Boolean  valueBoolean;
  public ValueObject object;
}

class KeyValueObject {
  public String key;
  public ValueObject value;
}

class ConsensusEvent {
  public Signature signatures;
  public Transaction transaction;
  public Long order;
  public String status;
}

class CommandOption {
  public ArrayList<KeyValueObject> optionList;
}

class Transaction {
  public String creatorPubKey;
  public String command;
  public CommandOption commandOption;
  public Object object;

  public ArrayList<Signature> signatures;
  public String tag;
  public String hash;
  public Long timestamp;
  public String message;
}

// Communication
class TransactionResponse {
  public String message;
  public Short  code;
  public ArrayList<Transaction> transaction;
}

class RecieverConfirmation {
  public Signature signature;
  public Long timestamp;
}

class QueryResponse {
  public ArrayList<Object> objects;
  public Short code;
  public Long timestamp;
  public Signature signature;
}

class PeerResponse {
  public String message;
  public Short code;
  public Long timestamp;
  public Signature signature;
}

class StatusResponse {
  public String value;
  public String message;
  public Long timestamp;
  public RecieverConfirmation confirm;
  public Signature signature;
}
