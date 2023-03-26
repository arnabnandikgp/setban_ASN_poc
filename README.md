# Setban_ASN_poc

This is a proof of concept for the proposed expansion of the setban RPC to process ASNs and ban them just like IP addresses. There are also some discussions here.

Design Decisions
---
1. Making a new binary file to store the banned ASNs into the new binary file in the likes of bannedlist.dat used for storing the banned IP addresses.
2. Will implement additional checks in the IsBanned method to allow bucketing for addrman.h to not select any peer belonging to a banned AS.
3. To implement the feature of disconnecting an already connected peer which has AS belonging to banned AS list I have chosen to use the methods in '   'Cconnman:: DisconnectNode' in order to ban any peer by iterating through the list of connected nodes and disconnecting them.

To take the input as an ASN into the setban RPC we need to make it clear that it is an ASN and not a IP address.
We can take motivation from the fact that the existing setban RPC takes into account that there are / in a subnet address and thus separates subnet masks and individual host addresses by checking whether there is a '/' in the input or not, as follows:

```C
bool isSubnet = false;

if (request.params[0].get_str().find('/') != std::string::npos)
isSubnet = true;
```

So we can take the input and make an additional test and make a flag variable like for example isAS and if there are no dots in the parsed string since ASNs do not contain any then we can come to the conclusion that it is an ASN rather than a host address or a subnet mask
As follows:


```C
if (request.params[0].get_str().find('.') != std::string::npos)
        isAS = true;
    else
        isIP = true;
```
I have made a bold design desicion to make a new loacal binary file to store the list of the banned ASNs.This proposal is a bit bold as I know the fact that it will not be easy to get all the bitcoin developer community to agreement on building a new local file to store the list of banned list of ASN.
But my design proposal of making a new bianary is also valid from the fact that each user will have a different set of banned ASNs and just like the banned addresses list which is stred in local as binary file we will require a similar arrangement for the ASNs too. Also there is a possibility of using the same binary file i.e bannedlist.dat which currently stores the list of banned addresses to lso store the list of banned ASNs. Though I am not sure how to read and write two different categories of data from a single binary file.

Neverthelss in which binary file we will be storing our list of banned ASNs we will then load the contents of it onto a map with name say called “m_banned_As” just like when bucketing for peers the addrman first loads the content of bannedlist.dat of banned IP addresses onto a map “m_banned”.

As discussed earlier since we need to make suitable changes in the “IsBanned: method in order to detect IP addresses that were not explicitly banned but belong to a banned ASN, and to facilitate it checking for the contents of the m_banned_As we can have an “IsBannedAs” method to check in the continents of the banned AS list,[here](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/banman.cpp#L55)

```C
bool BanMan::IsBanned(const CNetAddr& net_addr)
{
    // is it there is the banned list of ips
    auto current_time = GetTime();
    LOCK(m_cs_bannedas);
    for (const auto& it : m_bannedas) {
        CSubNet sub_net = it.first;
        CBanEntry ban_entry = it.second;

        if (current_time < ban_entry.nBanUntil && sub_net.Match(net_addr)) {
            return true;
        }
    }

    std::vector<uint8_t> ip = net_addr.GetGroup(); // what is the 
    uint32_t Asn=Interpret(asmap,ip);    //interpret function defined in asmap.cpp
    if(IsAsBanned(Asn))
    {
        return true;
    }
    return false;
}
bool BanMan::IsBannedAs(std::uint32_t Asn)
{
    // returns true if the asn is in the banned list
    // will iterarte over all entries of the bannedasn list and return true if there and false otherwise. 
}

```

To make my proposal plan of making a separate file for storing all the banned ASNs happen we need to make suitable API s for accessing the bannedaslist.dat from the disk as well as make functions to modify its content just like the bannnedlist.dat for regular addresses. I have set out the basic structures of the additional APIs we will be requiring in the [banman.h](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/banman.h) and [banman.cpp](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/banman.cpp) files.

Since in https://github.com/bitcoin/bitcoin/blob/master/src/util/asmap.cpp we have uint_32 datatype as output for the `Interpret` function hence it will be suitable to stick to it and use the uint_32 datatype when handling ASNs.
```C
uint32_t Asn;
```

Adding the following methods and objects in the Banman class in order to ban ASNs using the asmap provided like changing the .Isbanned() method to encompass functionalities like banning peers that have a given ASN.

```C
public:
    ~BanMan();
    BanMan(fs::path ban_file, CClientUIInterface* client_interface, int64_t default_ban_time);
    void BanAsn(const uint32_t Asn, int64_t ban_time_offset = 0, bool since_unix_epoch = false);
    void ClearBannedAs();
    bool IsAsBanned(const uint32_t Asn);
    bool UnbanAsn(const uint32_t Asn);
    void GetBannedAs(banmap_t& banmap);
    void DumpBanAslist();

private:
    void LoadBanAslist() EXCLUSIVE_LOCKS_REQUIRED(!m_cs_bannedas);
    bool BannedSAsetIsDirty();
    void SetBannedSetDirty(bool dirty = true);
    void SweepBannedAs() EXCLUSIVE_LOCKS_REQUIRED(m_cs_bannedas);
```

We also need to  ensure that after placing the ASN in the banned list if there are any peers belonging to that AS should be disconnected immediately.
For that we take motivations from the disconnectnode method used for immediatetly disconnecting a peer with a given address. I intend to implement the following in the setban RPC in [net.cpp](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/net.cpp#L111)

```C
 for (CNode *pnode : m_nodes)
            {
                if(Interpret(asmap,(pnode->addr).GetGroup())==stoi(request.params[0]))
                {
                    // LogPrint(BCLog::NET, "disconnect by subnet%s matched peer=%d; disconnecting\n", (fLogIPs ? strprintf("=%s", subnet.ToString()) : ""), pnode->GetId());
                    pnode->fDisconnect = true;
                }
            }
```

Checklist
---
Make sure the following are in the final implementation.  
- [ ] All the new methods described in the POC for the Banman class to use the proposed bannedaslist.dat
- [ ] Modifying the setban RPC to handle arguments in the form of ASNs
- [ ] Making sure that the after adding an ASN to the bannedAs list all the already connected peers belonging to that AS get disconnected immediately.


Doubts
---
Doubts:
1. Are there any checks to validate whether input ASN is valid or not, in the likes of methods like lookuphost and lookupsubnet for the IP addresses.
2. Is there any way trough which we may use the bannedlst.dat for also storing the banned ASNs.
