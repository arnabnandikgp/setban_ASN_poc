#include <bits/stdc++.h>
using namespace std;

int main()
// working towards adding the ASN to a hypothetical local memory location
// here is what the future setban rpc code will look like
//net.cpp ka setban rpc
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
        { // changes in the code of existing framework
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

            int64_t banTime = 0; // use standard bantime if not specified
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
        netAsn = stoi(request.params[0].get_str()) //changing the string input to integer datatype.
                                                // now checking for the input command to add or remove or to delete
            if (strCommand == "add")
        {
            // checking whether the asn is already banned or not
            if (banman.isBanned())
            {
                // message showing that it is already there
                throw JSONRPCError(RPC_CLIENT_NODE_ALREADY_ADDED, "Error: ASN is already banned");
            }
            int64_t banTime = 0; // use standard bantime if not specified
            if (!request.params[2].isNull())
                banTime = request.params[2].getInt<int64_t>();

            const bool absolute{request.params[3].isNull() ? false : request.params[3].get_bool()};

            if (absolute && banTime < GetTime())
            {
                throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: Absolute timestamp is in the past");
            }

            banman.BanAsn(Asn, banTime, absolute); // using ban or any other function to add the asn to the lsit of banned ASNs.
            if (node.connman)
            {
                node.connman->DisconnectNode(Asn); // adding utilities in disconnectnode to ban existing peerss from those ASs.
            }
            // ensure that the in the list of connected nodes there isnt a one that belongs to a banned AS.
            for (CNode *pnode : m_nodes)
            {
                if(Interpret(asmap,(pnode->addr).GetGroup())==stoi(request.params[0]))
                {
                    // LogPrint(BCLog::NET, "disconnect by subnet%s matched peer=%d; disconnecting\n", (fLogIPs ? strprintf("=%s", subnet.ToString()) : ""), pnode->GetId());
                    pnode->fDisconnect = true;
                }
            }
        }
        else if (strCommand == "remove")
        {

            if (!(isSubnet ? banman.Unban(Asn) : banman.Unban(Asn)))
            {
                throw JSONRPCError(RPC_CLIENT_INVALID_IP_OR_SUBNET, "Error: Unban failed. Requested address/subnet was not previously manually banned.");
            }
        }
        return UniValue::VNULL;
    }
}
