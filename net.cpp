#include <rpc/server.h>

#include <addrman.h>
#include <banman.h>
#include <chainparams.h>
#include <clientversion.h>
#include <core_io.h>
#include <net_permissions.h>
#include <net_processing.h>
#include <net_types.h> // For banmap_t
#include <netbase.h>
#include <node/context.h>
#include <policy/settings.h>
#include <rpc/blockchain.h>
#include <rpc/protocol.h>
#include <rpc/server_util.h>
#include <rpc/util.h>
#include <sync.h>
#include <timedata.h>
#include <util/strencodings.h>
#include <util/string.h>
#include <util/time.h>
#include <util/translation.h>
#include <validation.h>
#include <version.h>
#include <warnings.h>

#include <optional>

#include <univalue.h>

using node::NodeContext;

// here is what my proposed setban RPC code will look like with some new and modified methods
{
    uint32_t netAsn;

    NodeContext &node = EnsureAnyNodeContext(request.context);
    BanMan &banman = EnsureBanman(node);

    bool isAS = false;
    bool isIP = false;

    if (request.params[0].get_str().find('.') != std::string::npos)
        isAS = true;
    else
        isIP = true;

    if (isIP)
    {
        bool isSubnet = false;

        if (request.params[0].get_str().find('/') != std::string::npos)
            isSubnet = true;

        if (!isSubnet)
        {                                                       // changes in the code of existing framework
            CNetAddr resolved;
            LookupHost(request.params[0].get_str(), resolved, false);
            netAddr = resolved;
        }
        else
            LookupSubNet(request.params[0].get_str(), subNet);
        if (strCommand == "add")
        {
            if (isSubnet ? banman.IsBanned(subNet) : banman.IsBanned(netAddr))
            {
                throw JSONRPCError(RPC_CLIENT_NODE_ALREADY_ADDED, "Error: IP/Subnet already banned");
            }

            int64_t banTime = 0;                            // use standard bantime if not specified
            if (!request.params[2].isNull())
                banTime = request.params[2].getInt<int64_t>();

            const bool absolute{request.params[3].isNull() ? false : request.params[3].get_bool()};

            if (absolute && banTime < GetTime())
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: Absolute timestamp is in the past");
            }

            if (isSubnet)
            {
                banman.Ban(subNet, banTime, absolute);
                if (node.connman)
                {
                    node.connman->DisconnectNode(subNet);
                }
            }
            else
            {
                banman.Ban(netAddr, banTime, absolute);
                if (node.connman)
                {
                    node.connman->DisconnectNode(netAddr);
                }
            }
        }
        else if (strCommand == "remove")
        {
            if (!(isSubnet ? banman.Unban(subNet) : banman.Unban(netAddr)))
            {
                throw JSONRPCError(RPC_CLIENT_INVALID_IP_OR_SUBNET, "Error: Unban failed. Requested address/subnet was not previously manually banned.");
            }
        }
        return UniValue::VNULL;
    }

    else
    {
        uint32_t Asn = stoi(request.params[0].get_str()) // function to convert the input into a netAsn object
                                                         // now checking for the input command to add or remove or to delete
            if (strCommand == "add")
        {
            if (banman.isBanned(Asn))                    // checking whether the asn is already banned or not
            {
                
                throw JSONRPCError(RPC_CLIENT_NODE_ALREADY_ADDED, "Error: ASN is already banned");  // message showing that it is already there
            }
            int64_t banTime = 0;                         // use standard bantime if not specified
            if (!request.params[2].isNull())
                banTime = request.params[2].getInt<int64_t>();

            const bool absolute{request.params[3].isNull() ? false : request.params[3].get_bool()};

            if (absolute && banTime < GetTime())
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: Absolute timestamp is in the past");
            }

            banman.Ban(Asn, banTime, absolute);         // using ban to add the asn to the list of banned ASNs.
            if (node.connman)                           // ensuring that the in the list of connected nodes there isnt a one that belongs to a banned AS.
            {                                           //taken motivation from Cconnman::DisConnectnode 
               for (CNode *pnode : m_nodes)
            {
                if (Interpret(asmap, (pnode->addr).GetGroup()) == Asn)
                {
                    pnode->fDisconnect = true;
                }
            } 
            }            
        }
        else if (strCommand == "remove")
        {

            if (!(banman.IsBanned(Asn)))
            {
                throw JSONRPCError(RPC_CLIENT_INVALID_IP_OR_SUBNET, "Error: Unban failed. Requested address/subnet was not previously manually banned.");
            }
        }
        return UniValue::VNULL;
    }
}
