#include <banman.h>
#include <netaddress.h>
#include <node/interface_ui.h>
#include <sync.h>
#include <util/system.h>
#include <util/time.h>
#include <util/translation.h>
#include <asmap.cpp>

uint32_t Asn;
BanMan::BanMan(fs::path ban_file, CClientUIInterface *client_interface, int64_t default_ban_time)
    : m_client_interface(client_interface), m_ban_db(std::move(ban_file)), m_default_ban_time(default_ban_time)
{
    LoadBanlist();
    DumpBanlist();
} // remains same

BanMan::~BanMan()
{
    DumpBanAslist();
}
void BanMan::LoadBanlist() // remains same
{
    ...
}
void BanMan::GetBanned(banmap_t &banmap)
{
    // gets the data from the binary file
    // remains same
}
void BanMan::DumpBanlist()
{
    // function to save the bannedasn list into disk
    // remains same
}

void BanMan::ClearBanned()
{
    // clearing the bannedas list
    // saving it to memory
    // remains same
}

void BanMan::Ban(uint32_t Asn, int64_t ban_time_offset, bool since_unix_epoch) // analogous to implementation of ban for CSubNet objects
{
    CBanEntry ban_entry(GetTime());

    int64_t normalized_ban_time_offset = ban_time_offset;
    bool normalized_since_unix_epoch = since_unix_epoch;
    if (ban_time_offset <= 0)
    {
        normalized_ban_time_offset = m_default_ban_time;
        normalized_since_unix_epoch = false;
    }
    ban_entry.nBanUntil = (normalized_since_unix_epoch ? 0 : GetTime()) + normalized_ban_time_offset;

    {
        LOCK(m_cs_banned);
        if (m_banned[Asn].nBanUntil < ban_entry.nBanUntil)
        {
            m_banned[Asn] = ban_entry;
            m_is_dirty = true;
        }
        else
            return;
    }
    if (m_client_interface)
        m_client_interface->BannedListChanged();

    // store banlist to disk immediately
    DumpBanlist();
}

bool BanMan::Unban(uint32_t Asn)
{
    {
        LOCK(m_cs_banned);
        if (m_banned.erase(Asn) == 0)
            return false;
        m_is_dirty = true;
    }
    if (m_client_interface)
        m_client_interface->BannedListChanged();
    DumpBanlist(); // store banlist to disk immediately
    return true;
}

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

void BanMan::GetBanned(banmap_t &banmap)
{
    // return vector of curently banned ASNs
    // remains same
}

bool BanMan::BannedSetIsDirty()
{
    // flag to detect changes in banned list
    // remains same
}

void BanMan::SetBannedSetDirty(bool dirty)
{
    // reuse m_banned lock for the m_is_dirty flag
    //remains same 
}

void BanMan::SweepBanned()
{
    AssertLockHeld(m_cs_banned);

    int64_t now = GetTime();
    bool notify_ui = false;
    banmap_t::iterator it = m_banned.begin();
    while (it != m_banned.end())
    {
        CSubNet sub_net = (*it).first;//<--problem
        CBanEntry ban_entry = (*it).second;
        if (!sub_net.IsValid() || now > ban_entry.nBanUntil)
        {
            m_banned.erase(it++);
            m_is_dirty = true;
            notify_ui = true;
            LogPrint(BCLog::NET, "Removed banned node address/subnet: %s\n", sub_net.ToString());
        }
        else
        {
            ++it;
        }
    }

    // update UI
    if (notify_ui && m_client_interface)
    {
        m_client_interface->BannedListChanged();
    }
}

// I am not sure whether to implement functions like discouragedAsn and reated functions as I
// don't think that there is any need to have AS based weighting of connections based on whether
// previously some host in the AS connected properly or not as actions of a single node in an AS do
// define the nature of all the nodes in that AS.

bool BanMan::IsDiscouraged(uint32_t Asn)
{
    // discourages connecting to it by assging a discouraged score
}

void BanMan::Discourage(uint32_t Asn)
{
    // adding a node to discouraged node's list
}
