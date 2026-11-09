# Several commands in one app session.
#
# Every other test starts on a freshly launched app, where RAM is zeroed.
# These tests run SIGN after another key operation without restarting the app,
# which is how wallets use it (GET_PUBLIC_KEY to check the account, then SIGN;
# or several SIGNs in a row for multisig / delegated harvesting).

from contextlib import contextmanager

from ragger.backend import BackendInterface, RaisePolicy
from ragger.navigator.navigation_scenario import NavigateWithScenario
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PublicKey

from apps.symbol import SymbolClient, ErrorType
from utils import ROOT_SCREENSHOT_PATH, SYMBOL_PATH, load_transaction_from_file

# Non-aggregate transaction: the device signs the whole payload.
TRANSACTION_FILE = "transfer_transaction.json"
# Reuse the golden snapshots of the single-command tests: the screens are the same.
APPROVE_SNAPSHOTS = "test_sign_tx_accepted/transfer_transaction"
REJECT_SNAPSHOTS = "test_sign_tx_refused"


@contextmanager
def no_raise(backend: BackendInterface):
    # Read error statuses from the response instead of catching ExceptionRAPDU.
    # After an exception from an async exchange with UI navigation, the next
    # sync exchange in the same test never gets a reply from ragger
    # (the app itself does reply).
    previous = backend.raise_policy
    backend.raise_policy = RaisePolicy.RAISE_NOTHING
    try:
        yield
    finally:
        backend.raise_policy = previous


def sign_and_verify(client: SymbolClient, navigator: NavigateWithScenario) -> None:
    transaction, _ = load_transaction_from_file(TRANSACTION_FILE)
    with client.send_async_sign_message(SYMBOL_PATH, transaction):
        navigator.review_approve(ROOT_SCREENSHOT_PATH, APPROVE_SNAPSHOTS)
    response = client.get_async_response()
    assert response is not None and len(response.data) == 64

    pub_key = client.parse_get_public_key_response(
        client.send_get_public_key_non_confirm(SYMBOL_PATH).data)
    Ed25519PublicKey.from_public_bytes(pub_key).verify(response.data, transaction)


def test_sign_after_get_public_key(scenario_navigator: NavigateWithScenario):
    client = SymbolClient(scenario_navigator.backend)
    client.send_get_public_key_non_confirm(SYMBOL_PATH)
    sign_and_verify(client, scenario_navigator)


def test_sign_twice(scenario_navigator: NavigateWithScenario):
    client = SymbolClient(scenario_navigator.backend)
    sign_and_verify(client, scenario_navigator)
    sign_and_verify(client, scenario_navigator)


def test_sign_after_refused_sign(scenario_navigator: NavigateWithScenario):
    client = SymbolClient(scenario_navigator.backend)
    transaction, _ = load_transaction_from_file(TRANSACTION_FILE)
    with no_raise(scenario_navigator.backend):
        with client.send_async_sign_message(SYMBOL_PATH, transaction):
            scenario_navigator.review_reject(ROOT_SCREENSHOT_PATH, REJECT_SNAPSHOTS)
    assert client.get_async_response().status == ErrorType.TRANSACTION_REJECTED
    sign_and_verify(client, scenario_navigator)


# In this test we check that invalid sign data gives the same error every time (no UI, no snapshots)
def test_invalid_sign_status_is_stable(backend: BackendInterface):
    client = SymbolClient(backend)
    bad_payload = bytes(10)  # too short to be a transaction

    def bad_sign_status() -> int:
        with no_raise(backend):
            with client.send_async_sign_message(SYMBOL_PATH, bad_payload):
                pass
        return client.get_async_response().status

    assert bad_sign_status() == ErrorType.INVALID_SIGNING_DATA
    client.send_get_public_key_non_confirm(SYMBOL_PATH)
    assert bad_sign_status() == ErrorType.INVALID_SIGNING_DATA
    assert bad_sign_status() == ErrorType.INVALID_SIGNING_DATA
