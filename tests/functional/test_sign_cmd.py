from json import load
import pytest

from ragger.error import ExceptionRAPDU
from ragger.navigator.navigation_scenario import NavigateWithScenario
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey
from cryptography.exceptions import InvalidSignature

from apps.symbol import SymbolClient, ErrorType
from apps.symbol_transaction_builder import encode_txn_context
from utils import ROOT_SCREENSHOT_PATH, CORPUS_DIR, CORPUS_FILES

# Proposed XYM derivation paths for tests ###
SYMBOL_PATH = "m/44'/4343'/0'/0'/0'"


def load_transaction_from_file(transaction_filename: str) -> tuple[bytes,str]:
    with open(CORPUS_DIR / transaction_filename, encoding="utf-8") as f:
        transaction = load(f)
    return encode_txn_context(transaction), transaction['common_txn_header']['transactionType']


@pytest.mark.parametrize("transaction_filename", CORPUS_FILES)
def test_sign_tx_accepted(transaction_filename: str, scenario_navigator: NavigateWithScenario):
    transaction, transaction_type = load_transaction_from_file(transaction_filename)
    client = SymbolClient(scenario_navigator.backend)
    test_name = scenario_navigator.test_name + "/" + transaction_filename.replace(".json", "")
    with client.send_async_sign_message(SYMBOL_PATH, transaction):
        scenario_navigator.review_approve(ROOT_SCREENSHOT_PATH, test_name)

    # Verify signature
    response = client.get_async_response()
    assert response is not None
    assert len(response.data) == 64  # ED25519 signature is 64 bytes

    # Determine what data was actually signed by the device
    if transaction_type in ['AGGREGATE_COMPLETE', 'AGGREGATE_BONDED']:
        # For aggregate transactions, the device checks the generation hash based on BIP32 path
        # Path m/44'/4343'/... uses coin_type 4343 = Symbol mainnet
        TESTNET_GENERATION_HASH = bytes.fromhex(
            '49d6e1ce276a85b70eafe52349aacca389302e7a9754bcf1221e79494fc665a4'
        )
        MAINNET_GENERATION_HASH = bytes.fromhex(
            '57f7da205008026c776cb6aed843393f04cd458e0aa2d9f1d5f31a402072b2d6'
        )

        # The device determines expected hash from BIP32 path coin_type
        # SYMBOL_PATH = "m/44'/4343'/0'/0'/0'" -> coin_type 4343 -> mainnet
        is_using_mainnet = True  # coin_type 4343 = Symbol mainnet
        expected_generation_hash = MAINNET_GENERATION_HASH if is_using_mainnet else TESTNET_GENERATION_HASH

        if transaction[:32] == expected_generation_hash:
            # Sign 88 bytes: generation hash (32) + header (36) + fee (16) + payloadSize (4)
            signed_data = transaction[:88]
        else:
            # Cosigning - sign only the 32-byte transaction hash
            signed_data = transaction[:32]
    else:
        # For non-aggregate transactions, sign all data
        signed_data = transaction

    # Get public key for signature verification
    pub_key_response = client.send_get_public_key_non_confirm(SYMBOL_PATH).data
    public_key_bytes = client.parse_get_public_key_response(pub_key_response)
    # Verify the signature with the public key
    public_key = Ed25519PublicKey.from_public_bytes(public_key_bytes)
    try:
        public_key.verify(response.data, signed_data)
    except InvalidSignature:
        pytest.fail("Invalid signature returned by device")


def test_sign_tx_refused(scenario_navigator: NavigateWithScenario):
    transaction, _ = load_transaction_from_file("transfer_transaction.json")
    client = SymbolClient(scenario_navigator.backend)

    try:
        with client.send_async_sign_message(SYMBOL_PATH, transaction):
            scenario_navigator.review_reject(ROOT_SCREENSHOT_PATH)
    except ExceptionRAPDU as e:
        assert e.status == ErrorType.TRANSACTION_REJECTED
