#include <banman.h>
#include <netaddress.h>
#include <node/interface_ui.h>
#include <sync.h>
#include <util/system.h>
#include <util/time.h>
#include <util/translation.h>
#include <asmap.cpp>
int main()
{
uint32_t Asn; 
BanMan::BanMan(fs::path ban_file, CClientUIInterface* client_interface, int64_t default_ban_time)
    : m_client_interface(client_interface), m_ban_db(std::move(ban_file)), m_default_ban_time(default_ban_time)
{
    LoadBanlist();
    DumpBanlist();
}

BanMan::~BanMan()
{
    DumpBanAslist();
}
void BanMan::LoadBanAslist()
{
    //loads the list of ASN that are banned
}
void BanMan::GetBannedAs(banmap_t& banmap)
{
    LOCK(m_cs_banned);
    // Sweep the banlist so expired bans are not returned
    SweepBanned();
    banmap = m_bannedas; //create a thread safe copy
}
void BanMan::DumpBanAslist()
{
    //function to save the bannedasn list into disk
}

void BanMan::ClearBannedAs()
{
    //clearing the bannedas list
    //saving it to memory
}

void BanMan::BanAsn(uint32_t Asn)
{
    // adding new ASNs into the list
}

bool BanMan::UnbanAsn(uint32_t Asn)
{
   // removes the ASN from banlist 
}

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

void BanMan::GetBannedAsn(banmap_t& banmap)
{
    // return vector of curently banned ASNs 
}

void BanMan::SweepBannedAs()
{
    // for removing expired entries based on elapsed time criterion
}

bool BanMan::BannedAsSetIsDirty()
{
     // flag to detect changes in banned list  
}

void BanMan::SetBannedAsSetDirty(bool dirty)
{
    LOCK(m_cs_banned); //reuse m_banned lock for the m_is_dirty flag
    mas_is_dirty = dirty;
}
}

// i am not sure whether to implement functions like discouragedAsn and reated functions as 
// i am not sure about the need to have AS based weighting of connections based on whether
// previously some host in the AS connected properly or not as actions of a single node in an AS do
// define the nature of all the nodes in that AS.

bool BanMan::IsDiscouragedAs(uint32_t Asn)
{
    //discourages connecting to it by assging a discouraged score
}

void BanMan::DiscourageAsn(uint32_t Asn)
{
    // adding a node to discouraged node's list
}



