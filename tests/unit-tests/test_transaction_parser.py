#!/usr/bin/env python3

import sys
import json

from pathlib import Path
from subprocess import run


SYMBOL_LIB_DIRECTORY = (Path(__file__).resolve().parent.parent / "functional").resolve().as_posix()
sys.path.append(SYMBOL_LIB_DIRECTORY)
from apps.symbol_transaction_builder import encode_txn_context

CORPUS_DIR = Path(__file__).resolve().parent.parent / "corpus"
PARSER_BINARY = (Path(__file__).parent / "build/test_transaction_parser").resolve().as_posix()
TEMP_TXN_FILE = (Path(__file__).parent / "temp_txn.raw").resolve().as_posix()


TESTS_CASES = {
    "transfer_transaction.json": [
        ("Transaction Type", "Transfer"),
        ("Recipient", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Amount", "45 XYM"),
        ("Message Type", "Plain text"),
        ("Message", "This is a test message"),
        ("Fee", "2 XYM")
    ],
    "transfer_transaction_not_xym.json": [
        ("Transaction Type", "Transfer"),
        ("Recipient", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Mosaics", "Found 1"),
        ("Unknown Mosaic", "Divisibility and levy cannot be shown"),
        ("Amount", "45000000 micro 0x5E62990DCAC5B21A"),
        ("Message Type", "Plain text"),
        ("Message", "This is a test message"),
        ("Fee", "2 XYM")
    ],
    "create_mosaic.json": [
        ("Transaction Type", "Aggregate Complete"),
        ("Agg. Tx Hash", "E5F37FE3F83F4F0A2F21E7CF25F75CF29A20D7929CBEB7EB552EDA846969281F"),
        ("Inner TX Type", "Mosaic definition"),
        ("Mosaic ID", "532CB823113F2471"),
        ("Divisibility", "0"),
        ("Duration", "0d 0h 5m"),
        ("Transferable", "Yes"),
        ("Supply Mutable", "Yes"),
        ("Restrictable", "Yes"),
        ("Inner TX Type", "Mosaic Supply Change"),
        ("Mosaic ID", "532CB823113F2471"),
        ("Change Direction", "Increase"),
        ("Change Amount", "1000000"),
        ("Fee", "2 XYM"),
    ],
    "create_namespace.json": [
        ("Transaction Type", "Namespace Registration"),
        ("Namespace Type", "Root namespace"),
        ("Name", "foo576sgnlxdnfbdx"),
        ("Duration", "60d 0h 0m"),
        ("Fee", "2 XYM"),
    ],
    "create_sub_namespace.json": [
        ("Transaction Type", "Namespace Registration"),
        ("Namespace Type", "Sub namespace"),
        ("Name", "foo576sgnlxdnfbdx"),
        ("Parent ID", "000000000002A300"),
        ("Fee", "2 XYM"),
    ],
    "supply_change_mosaic.json": [
        ("Transaction Type", "Mosaic Supply Change"),
        ("Mosaic ID", "7CDF3B117A3C40CC"),
        ("Change Direction", "Increase"),
        ("Change Amount", "1000000"),
        ("Fee", "2 XYM")
    ],
    "link_namespace_to_mosaic.json": [
        ("Transaction Type", "Mosaic Alias"),
        ("Alias Type", "Unlink address"),
        ("Namespace ID", "82A9D1AC587EC054"),
        ("Mosaic ID", "7CDF3B117A3C40CC"),
        ("Fee", "2 XYM")
    ],
    "link_namespace_to_address.json": [
        ("Transaction Type", "Address Alias"),
        ("Alias Type", "Link address"),
        ("Namespace ID", "82A9D1AC587EC054"),
        ("Address", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Fee", "2 XYM")
    ],
    "account_address_restriction.json": [
        ("Transaction Type", "Account Address Restriction"),
        ("Addition Count", "1 address(es)"),
        ("Address", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Deletion Count", "Not change"),
        ("Restriction Flag", "Block"),
        ("Restriction Flag", "Imcoming"),
        ("Restriction Flag", "Address"),
        ("Fee", "0.16 XYM")
    ],
    "account_mosaic_restriction.json": [
        ("Transaction Type", "Account Mosaic Restriction"),
        ("Addition Count", "1 mosaic(s)"),
        ("Mosaic ID", "5BA212858B2B48BC"),
        ("Deletion Count", "Not change"),
        ("Restriction Flag", "Block"),
        ("Restriction Flag", "Mosaic"),
        ("Fee", "0.144 XYM")
    ],
    "account_operation_restriction.json": [
        ("Transaction Type", "Account Operation Restriction"),
        ("Addition Count", "1 operation(s)"),
        ("Operation Type", "Account Key Link"),
        ("Deletion Count", "Not change"),
        ("Restriction Flag", "Block"),
        ("Restriction Flag", "Outgoing"),
        ("Restriction Flag", "Transaction Type"),
        ("Fee", "0.138 XYM")
    ],
    "account_multisig.json": [
        ("Transaction Type", "Aggregate Bonded"),
        ("Agg. Tx Hash", "043D6F6E851CAE4ED2B975AEEF61DFDF00B85BBB2503AC23DD7586E3C0B07956"),
        ("Inner TX Type", "Multisig Account Modification"),
        ("Address Add Num", "2"),
        ("Address", "TALSLGUUF5VOB2RSWAPDNBUHIBKTNZQREXWPOAI"),
        ("Address", "TBFXGDVDW4TMYEVJ7L3YWTJXGVH7Q4RNXOKQCNY"),
        ("Address Del Num", "0"),
        ("Min Approval", "Add 1 address(es)"),
        ("Min Removal", "Add 1 address(es)"),
        ("Fee", "2 XYM")
    ],
    "hash_lock_account_multisig.json": [
        ("Transaction Type", "Funds Lock"),
        ("Lock Quantity", "10 XYM"),
        ("Duration", "0d 4h 0m"),
        ("Tx Hash", "2B51EBCBC3E40EFE8AF68A0408F5A72474B1327A64E3E3B47D9B139230C7833B"),
        ("Fee", "2 XYM")
    ],
    "multisig_transfer_transaction.json": [
        ("Transaction Type", "Aggregate Bonded"),
        ("Agg. Tx Hash", "4941C270B56778E01629FC82EDDC622668F076CE1583AFCCA3F6DE7FE03615BB"),
        ("Detail TX Type", "Transfer"),
        ("Recipient", "TBKQPST7HUOJA2PBNYNA7TT4LLKGA5BB5UY6M4Y"),
        ("Amount", "10 XYM"),
        ("Message Type", "Plain text"),
        ("Message", "Test message"),
        ("Fee", "0.03024 XYM"),
    ],
    "multisig_create_mosaic.json": [
        ("Transaction Type", "Aggregate Bonded"),
        ("Agg. Tx Hash", "705B456E99A2FA7DA3D4F02ABB1993774426B8095705C2116E6FB59E95A2587D"),
        ("Inner TX Type", "Mosaic definition"),
        ("Mosaic ID", "78CA2F4797C65A64"),
        ("Divisibility", "0"),
        ("Duration", "Unlimited"),
        ("Transferable", "Yes"),
        ("Supply Mutable", "Yes"),
        ("Restrictable", "No"),
        ("Inner TX Type", "Mosaic Supply Change"),
        ("Mosaic ID", "78CA2F4797C65A64"),
        ("Change Direction", "Increase"),
        ("Change Amount", "500000000"),
        ("Fee", "0.033696 XYM"),
    ],
    "multisig_create_namespace.json": [
        ("Transaction Type", "Aggregate Bonded"),
        ("Agg. Tx Hash", "B96E1C08F8434BFDC4D1F292EB3F911B1A3C5B3EE102887A8ACDD75A79A4BB62"),
        ("Inner TX Type", "Namespace Registration"),
        ("Namespace Type", "Root namespace"),
        ("Name", "multisig"),
        ("Duration", "60d 0h 0m"),
        ("Fee", "0.026784 XYM")
    ],
    "hash_lock_multisig_create_namespace.json": [
        ("Transaction Type", "Funds Lock"),
        ("Lock Quantity", "10 XYM"),
        ("Duration", "0d 8h 20m"),
        ("Tx Hash", "E019A4A92002505B8B5029AE556958ADCDFBEDAC26C2F79DE1668C5BC588EDF7"),
        ("Fee", "0.019872 XYM"),
    ],
    "multisig_create_sub_namespace.json": [
        ("Transaction Type", "Namespace Registration"),
        ("Namespace Type", "Sub namespace"),
        ("Name", "sub_namespace_multisig"),
        ("Parent ID", "D64FAC0976CC0914"),
        ("Fee", "0.018144 XYM")
    ],
    "cosignature_transaction.json": [
        ("Transaction Type", "Aggregate Bonded"),
        ("Agg. Tx Hash", "0EFE6E4A881D312984767CABBE53DAC00419E179932A5C784B51132FBE5F7C88"),
        ("Detail TX Type", "Transfer"),
        ("Recipient", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Amount", "10 XYM"),
        ("Message Type", "Plain text"),
        ("Message", "SDV"),
        ("Fee", "0.48 XYM")
    ],
    "account_metadata_transaction.json": [
        ("Transaction Type", "Aggregate Complete"),
        ("Agg. Tx Hash", "5F221AD2C6D297E683692CE332B24157057E6FB43A832F18C13495EC49544E08"),
        ("Inner TX Type", "Account Metadata"),
        ("Target Address", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Metadata Key", "AB8385A30DFCEA7A"),
        ("Value", "this is the value field of account metadata"),
        ("Value Size Delta", "Increase 43 byte(s)"),
        ("Fee", "0.296 XYM")
    ],
    "mosaic_metadata_transaction.json": [
        ("Transaction Type", "Aggregate Complete"),
        ("Agg. Tx Hash", "FD62E4D107693B6B0A7D862F2BBE49695565764AE41AE0D0344C47AE82DCB00C"),
        ("Inner TX Type", "Mosaic Metadata"),
        ("Target Address", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Mosaic ID", "6E32F5200421C596"),
        ("Metadata Key", "D00C0B75EFB5FA9F"),
        ("Value", "This is the mosaic metadata value field"),
        ("Value Size Delta", "Increase 39 byte(s)"),
        ("Fee", "0.304 XYM")
    ],
    "namespace_metadata_transaction.json": [
        ("Transaction Type", "Aggregate Complete"),
        ("Agg. Tx Hash", "668FE1351AC31C35536EE3A368F2C2310DD3D7E67A8345050548AE8B6596015D"),
        ("Inner TX Type", "Namespace Metadata"),
        ("Target Address", "TDZKL2HAMOWRVEEF55NVCZ7C6GSWIXCI7IWAESI"),
        ("Namespace ID", "8547528FC63C2AD6"),
        ("Metadata Key", "9E828FFAA77C9D6D"),
        ("Value", "Namespace metadata value field"),
        ("Value Size Delta", "Increase 30 byte(s)"),
        ("Fee", "0.296 XYM")
    ],
    "delegated_harvesting.json": [
        ("Transaction Type", "Aggregate Complete"),
        ("Agg. Tx Hash", "0C8666CEF61F61B78515149A1414455C77A0CCD7C9AD5F39DFE46761AB6556DF"),
        ("Inner TX Type", "Account Key Link"),
        ("Action", "Link"),
        ("Linked Acct. PbK", "00278C080D6B149902E1576723DA6362065D3A134BEE6383827353540492B911"),
        ("Inner TX Type", "Vrf Key Link"),
        ("Action", "Link"),
        ("Linked Vrf PbK", "C1A71431325873D83894977C68C783A4EED7A3391EAFD704BF8500361EC321DB"),
        ("Inner TX Type", "Node Key Link"),
        ("Action", "Link"),
        ("Linked Node PbK", "81890592F960AAEBDA7612C8917FA9C267A845D78D74D4B3651AF093E6775001"),
        ("Fee", "0.0432 XYM")
    ],
    "persistent_harvesting_delegation_transfer.json": [
        ("Transaction Type", "Transfer"),
        ("Recipient", "TBR6AUIUNBRSXJ34RSCZOJTK4GQNVDEUEDCPMIY"),
        ("Message Type", "Persistent harvesting delegation"),
        ("Harvesting Message", "FE2A8061577301E28AEC26D42EFCE832BE498BB8CFCC7687BC5BC6B22A82F4BA415A7DF13E1DEA994EAD70125CA250DD6CD8AEA8BAE26AD9A8FC9CB45A996E59BD8894E3D618043887E2383A6BB161A18AB58F406D7DFF384CBD6A669FD152E5AD84B372425212CAAECCB712674AA6C737894BB14FADFE93A3E3AF73A34187D49740891C"),
        ("Fee", "0.292 XYM")
    ]
}


def assert_equal(a, b, text):
    if a != b:
        print(f"[  ERROR   ] Mismatch in {text}: <{a}> vs <{b}>")
        return False
    return True


def test_parsing(filename, expected):
    print("[ RUN      ] ", filename)
    with open(CORPUS_DIR / filename) as f:
        transaction = json.load(f)

    tx_data = encode_txn_context(transaction)

    with open(TEMP_TXN_FILE, 'wb') as f:
        f.write(tx_data)

    cmd = [PARSER_BINARY, TEMP_TXN_FILE]
    res = run(cmd, capture_output=True)
    status = res.returncode

    if status != 0:
        print("[  ERROR   ] ", res.stderr)
    else:
        parsed = res.stdout.decode().strip()
        received = [pair.split("::") for pair in parsed.split("\n")]
        if not assert_equal(len(received), len(expected), "number of fields"):
            status = 1
        else:
            for i in range(len(received)):
                if not assert_equal(received[i][0], expected[i][0], "name of field"):
                    status = 1
                    break
                if not assert_equal(received[i][1], expected[i][1], "value of field"):
                    status = 1
                    break

    if status != 0:
        print("[  FAILED  ] ", filename)
    else:
        print("[       OK ] ", filename)
    return status


status = 0
for filename, expected in TESTS_CASES.items():
    res = test_parsing(filename, expected)
    if res != 0:
        status = res

exit(status)
