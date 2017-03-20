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
import Object;
import Primitives;

public class Command {}

public class Add extends Command {
  public Object object;
}

public class Remove extends Command {
  public Object object;
}

public class Transfer extends Command {
  public Asset currency;
  public PublicKey sender;
  public PublicKey receiver;
}

public class Execute extends Command {
  public String name;
}

public class SetPeerTrust extends Command {
  public PublicKey peerPubKey;
  public Double    trust;
}

public class ChangePeerTrust extends Command {
  public PublicKey peerPubKey;
  public Double    delta;
}

public class SetPeerPermission extends Command {
  public PublicKey peerPubKey;
  public PeerPermission permission;
}

public class SetChaincode extends Command {
  public Chaincode chaincode;
}

//////////////////////////////////////////
/// Commands for Account
public class AddSignatory {
  public PublicKey account;
  public List<PublicKey> signatory;
}

public class SetAccountsUseKeys {
  public List<PublicKey> accounts;
  public Short useKeys;
}
//////////////////////////////////////////
