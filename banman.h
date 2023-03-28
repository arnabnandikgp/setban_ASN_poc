#ifndef BITCOIN_BANMAN_H
#define BITCOIN_BANMAN_H

#include <addrdb.h>
#include <common/bloom.h>
#include <fs.h>
#include <net_types.h> // For banmap_t
#include <sync.h>

#include <chrono>
#include <cstdint>
#include <memory>

// NOTE: When adjusting this, update rpcnet:setban's help ("24h")
static constexpr unsigned int DEFAULT_MISBEHAVING_BANTIME = 60 * 60 * 24; // Default 24-hour ban

/// How often to dump banned addresses/subnets to disk.
static constexpr std::chrono::minutes DUMP_BANS_INTERVAL{15};

class CClientUIInterface;
class CNetAddr;
class CSubNet;
class BanMan;

{
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

    RecursiveMutex m_cs_banned;
    banmap_t mas_banned GUARDED_BY(mas_cs_banned);
    bool mas_is_dirty GUARDED_BY(mas_cs_banned){false};
    CClientUIInterface* mas_client_interface = nullptr;
    CBanDB mas_ban_db;
    const int64_t mas_default_ban_time;
    // CRollingBloomFilter m_discouraged GUARDED_BY(m_cs_banned) {50000, 0.000001};
};

#endif // BITCOIN_BANMAN_H