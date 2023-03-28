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
Just like how singulat host addresses and subnetmasks are stored together into the bannedlist.dat binary file, we can also include the ASN mappings in it too. Hence we will be using the same local binary file to store the list of banned IP addresses as well as ASNs.

As discussed earlier since we need to make suitable changes in the “IsBanned" method in order to detect IP addresses that were not explicitly banned but belong to a banned ASN [here](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/banman.cpp#L55)

```C
bool BanMan::IsBanned(const uint32_t &Asn) // again analogous to CSubNet object implementation
{
    auto current_time = GetTime();
    LOCK(m_cs_banned);
    banmap_t::iterator i = m_banned.find(Asn);
    if (i != m_banned.end())
    {
        CBanEntry ban_entry = (*i).second;
        if (current_time < ban_entry.nBanUntil)
        {
            return true;
        }
    }
    return false;
}

```

To make my proposal plan of using the same binary  file for storing all the banned ASNs happen we need to make suitable API s for accessing the bannedlist.dat from the disk as well as make functions to modify its content. I have set out the basic structures of the additional APIs we will be requiring in the [banman.h](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/banman.h) and [banman.cpp](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/banman.cpp) files.

Since in https://github.com/bitcoin/bitcoin/blob/master/src/util/asmap.cpp we have uint_32 datatype as output for the `Interpret` function hence it will be suitable to stick to it and use the uint_32 datatype when handling ASNs.
```C
uint32_t Asn;
```

Adding the following methods and objects in the Banman class in order to ban ASNs using the asmap provided like changing the .Isbanned() method to encompass functionalities like banning peers that have a given ASN.

```C
public:
    ~BanMan();
    BanMan(fs::path ban_file, CClientUIInterface* client_interface, int64_t default_ban_time);
    void Ban(const uint32_t Asn, int64_t ban_time_offset=0 , bool since_unix_epoch = false);
    void ClearBanned();
    bool IsBanned(const uint32_t Asn);
    bool Unban(const uint32_t Asn);
    void GetBanned(banmap_t& banmap);
    void DumpBanlist();

private:
    void LoadBanlist() EXCLUSIVE_LOCKS_REQUIRED(!m_cs_bannedas);
    bool BannedSetIsDirty();
    void SetBannedSetDirty(bool dirty = true);
    void SweepBanned() EXCLUSIVE_LOCKS_REQUIRED(m_cs_bannedas);
```

We also need to  ensure that after placing the ASN in the banned list if there are any peers belonging to that AS should be disconnected immediately.
For that we take motivations from the [disconnectnode](https://doxygen.bitcoincore.org/net_8cpp_source.html#l02853) method used for immediatetly disconnecting a peer with a given address. I intend to implement the following in the setban RPC in [net.cpp](https://github.com/arnabnandikgp/setban_ASN_poc/blob/main/net.cpp#L111)

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
- [ ] Making sure that after adding an ASN to the bannedAs list all the already connected peers belonging to that AS get disconnected immediately.


Doubts
---
Doubts:
1. Are there any checks to validate whether input ASN is valid or not, in the likes of methods like lookuphost and lookupsubnet for the IP addresses.
2. Is there any way trough which we may use the bannedlst.dat for also storing the banned ASNs.
