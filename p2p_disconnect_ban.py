"""Test node disconnect and ban behavior"""
import time

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import (
    assert_equal,
    assert_raises_rpc_error,
)

class DisconnectBanTest(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 2
        self.supports_cli = False

    def run_test(self):
        self.log.info("Connect nodes both way")
        # By default, the test framework sets up an addnode connection from
        # node 1 --> node0. By connecting node0 --> node 1, we're left with
        # the two nodes being connected both ways.
        # Topology will look like: node0 <--> node1
        self.connect_nodes(0, 1)  

        self.log.info("Test setban and listbanned RPCs")

        self.log.info("setban: successfully ban single AS")
        assert_equal(len(self.nodes[1].getpeerinfo()), 2)  # node1 should have 2 connections to node0 at this point
        self.nodes[1].setban("12345678", command="add")#adding ASN of node0
        self.wait_until(lambda: len(self.nodes[1].getpeerinfo()) == 0, timeout=10)
        assert_equal(len(self.nodes[1].getpeerinfo()), 0)  # all nodes must be disconnected at this point
        assert_equal(len(self.nodes[1].listbanned()), 1)

        self.log.info("setban: succesfully ban single IP address belonging to a banned AS")
        self.nodes[1].setban(subnet="127.0.0.1", command="add",)
        assert_equal(len(self.nodes[1].listbanned()), 2)
        
        #need to include a test that will depend on the design decisions made during the project of whether to ban an IP while it's AS is already banned.

        self.log.info("setban: fail to ban  AS with past absolute timestamp")
        assert_raises_rpc_error(error_code, "Error: Absolute timestamp is in the past", self.nodes[1].setban, "12345678", "add", 123, True)

        self.log.info("setban remove: fail to unban a non-banned AS")
        assert_raises_rpc_error(error_code, "Error: Unban failed", self.nodes[1].setban, "1245668", "remove")
        self.nodes[1].setban("12345678", command="add")
        assert_equal(len(self.nodes[1].listbanned()), 1)

        self.log.info("setban remove: successfully unban AS")
        self.nodes[1].setban("1245678", "remove")
        assert_equal(len(self.nodes[1].listbanned()), 0)
        self.nodes[1].clearbanned()
        assert_equal(len(self.nodes[1].listbanned()), 0)


if __name__ == '__main__':
    DisconnectBanTest().main()
