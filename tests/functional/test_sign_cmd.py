from json import load
import pytest

from ragger.error import ExceptionRAPDU
from ragger.navigator.navigation_scenario import NavigateWithScenario

from apps.symbol import SymbolClient, ErrorType
from apps.symbol_transaction_builder import encode_txn_context
from utils import ROOT_SCREENSHOT_PATH, CORPUS_DIR, CORPUS_FILES

# Proposed XYM derivation paths for tests ###
SYMBOL_PATH = "m/44'/4343'/0'/0'/0'"


def load_transaction_from_file(transaction_filename: str) -> bytes:
    with open(CORPUS_DIR / transaction_filename, encoding="utf-8") as f:
        transaction = load(f)
    return encode_txn_context(transaction)


@pytest.mark.parametrize("transaction_filename", CORPUS_FILES)
def test_sign_tx_accepted(transaction_filename: str, scenario_navigator: NavigateWithScenario):
    transaction = load_transaction_from_file(transaction_filename)
    client = SymbolClient(scenario_navigator.backend)
    test_name = scenario_navigator.test_name + "/" + transaction_filename.replace(".json", "")
    with client.send_async_sign_message(SYMBOL_PATH, transaction):
        scenario_navigator.review_approve(ROOT_SCREENSHOT_PATH, test_name)
    # Missing signature verification


def test_sign_tx_refused(scenario_navigator: NavigateWithScenario):
    transaction = load_transaction_from_file("transfer_transaction.json")
    client = SymbolClient(scenario_navigator.backend)

    try:
        with client.send_async_sign_message(SYMBOL_PATH, transaction):
            scenario_navigator.review_reject(ROOT_SCREENSHOT_PATH)
    except ExceptionRAPDU as e:
        assert e.status == ErrorType.TRANSACTION_REJECTED
