// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Era developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "assert.h"

#include "chainparams.h"
#include "main.h"
#include "util.h"

#include <boost/assign/list_of.hpp>

using namespace boost::assign;

struct SeedSpec6 {
    uint8_t addr[16];
    uint16_t port;
};

#include "chainparamsseeds.h"

//
// Main network
//

// Convert the pnSeeds6 array into usable address objects.
static void convertSeed6(std::vector<CAddress> &vSeedsOut, const SeedSpec6 *data, unsigned int count)
{
    // It'll only connect to one or two seed nodes because once it connects,
    // it'll get a pile of addresses with newer timestamps.
    // Seed nodes are given a random 'last seen time' of between one and two
    // weeks ago.
    const int64_t nOneWeek = 7*24*60*60;
    for (unsigned int i = 0; i < count; i++)
    {
        struct in6_addr ip;
        memcpy(&ip, data[i].addr, sizeof(ip));
        CAddress addr(CService(ip, data[i].port));
        addr.nTime = GetTime() - GetRand(nOneWeek) - nOneWeek;
        vSeedsOut.push_back(addr);
    }
}

class CMainParams : public CChainParams {
public:
    CMainParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x61;
        pchMessageStart[1] = 0x32;
        pchMessageStart[2] = 0x70;
        pchMessageStart[3] = 0x3a;
        nDefaultPort = 13546;
        nRPCPort = 13547;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 16);

        const char* pszTimestamp = "Bittrex Bars Users Residing in 5 Countries Under US Embargo | JP Buntinx | The Merkle | March 6, 2018";
        std::vector<CTxIn> vin;
        vin.resize(1);
        vin[0].scriptSig = CScript() << 0 << CBigNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        std::vector<CTxOut> vout;
        vout.resize(1);
        vout[0].SetEmpty();
        CTransaction txNew(1, 1520366800, vin, vout, 0);
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock = 0;
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 1;
        genesis.nTime    = 1520366800;
        genesis.nBits    = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce   = 53683;

       /*
        Hashed MainNet Genesis Block Output
        block.hashMerkleRoot == 22a2e4e28386bf75759112222b20cf3b889cd60f5e55d7184f8dbdec4d8dc6ff
        block.nTime = 1520366800
        block.nNonce = 53683
        block.GetHash = 0000aab7dff29b0749519a7886b8a8d3f2806eb5dd861f9a0dbb7441f9a97f6a
       */

        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x0000aab7dff29b0749519a7886b8a8d3f2806eb5dd861f9a0dbb7441f9a97f6a"));
        assert(genesis.hashMerkleRoot == uint256("0x22a2e4e28386bf75759112222b20cf3b889cd60f5e55d7184f8dbdec4d8dc6ff"));

        vSeeds.push_back(CDNSSeedData("seed1", "64.150.189.104"));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 34);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 62);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1, 83);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x1f)(0x46)(0x20)(0x9f).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x1f)(0x88)(0x95)(0xbf).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_main, ARRAYLEN(pnSeed6_main));

        nLastPOWBlock = 0x7fffffff;
    }

    virtual const CBlock& GenesisBlock() const { return genesis; }
    virtual Network NetworkID() const { return CChainParams::MAIN; }

    virtual const vector<CAddress>& FixedSeeds() const {
        return vFixedSeeds;
    }
protected:
    CBlock genesis;
    vector<CAddress> vFixedSeeds;
};
static CMainParams mainParams;


//
// Testnet
//

class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        // The message start string is designed to be unlikely to occur in normal data.
        // The characters are rarely used upper ASCII, not valid as UTF-8, and produce
        // a large 4-byte int at any alignment.
        pchMessageStart[0] = 0x05;
        pchMessageStart[1] = 0x6c;
        pchMessageStart[2] = 0x9c;
        pchMessageStart[3] = 0x70;
        bnProofOfWorkLimit = CBigNum(~uint256(0) >> 14);
        nDefaultPort = 23536;
        nRPCPort = 23537;
        strDataDir = "testnet";

        // Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nBits  = bnProofOfWorkLimit.GetCompact();
        genesis.nNonce = 13731;

       /*
        Hashed TestNet Genesis Block Output
        block.hashMerkleRoot == 22a2e4e28386bf75759112222b20cf3b889cd60f5e55d7184f8dbdec4d8dc6ff
        block.nTime = 1520366800
        block.nNonce = 13731
        block.GetHash = 000149d0c0dec3468068735e9f9ccdd6d329ed2f6f60add7459773e502f067c5
       */
        
        hashGenesisBlock = genesis.GetHash();
        assert(hashGenesisBlock == uint256("0x000149d0c0dec3468068735e9f9ccdd6d329ed2f6f60add7459773e502f067c5"));

        vFixedSeeds.clear();
        vSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 32);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 63);
        base58Prefixes[SECRET_KEY]     = std::vector<unsigned char>(1, 145);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x82)(0x3a)(0xa0)(0x03).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x82)(0x3a)(0x06)(0x97).convert_to_container<std::vector<unsigned char> >();

        convertSeed6(vFixedSeeds, pnSeed6_test, ARRAYLEN(pnSeed6_test));

        nLastPOWBlock = 0x7fffffff;
    }
    virtual Network NetworkID() const { return CChainParams::TESTNET; }
};
static CTestNetParams testNetParams;




static CChainParams *pCurrentParams = &mainParams;

const CChainParams &Params() {
    return *pCurrentParams;
}

void SelectParams(CChainParams::Network network) {
    switch (network) {
        case CChainParams::MAIN:
            pCurrentParams = &mainParams;
            break;
        case CChainParams::TESTNET:
            pCurrentParams = &testNetParams;
            break;

        default:
            assert(false && "Unimplemented network");
            return;
    }
}

bool SelectParamsFromCommandLine() {

    bool fTestNet = GetBoolArg("-testnet", false);



    if (fTestNet) {
        SelectParams(CChainParams::TESTNET);
    } else {
        SelectParams(CChainParams::MAIN);
    }
    return true;
}
