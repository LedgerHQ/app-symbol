from ragger.backend import BackendInterface, SpeculosBackend
from ragger.navigator.navigation_scenario import NavigateWithScenario
from ragger.error import ExceptionRAPDU

from apps.symbol import SymbolClient, ErrorType
from utils import ROOT_SCREENSHOT_PATH

# Proposed XYM derivation paths for tests ###
SYMBOL_PATH = "m/44'/4343'/0'/0'/0'"

SPECULOS_EXPECTED_PUBLIC_KEY = "73f0bf90d39d1d0a3ec03740eec95c12"\
                               "7a82a20bf07f6840462125a94a42df1e"


def check_get_public_key_resp(backend: BackendInterface, public_key: bytes):
    if isinstance(backend, SpeculosBackend):
        # Check against nominal Speculos seed expected results
        assert public_key.hex() == SPECULOS_EXPECTED_PUBLIC_KEY


def test_get_public_key_non_confirm(backend: BackendInterface):
    client = SymbolClient(backend)
    response = client.send_get_public_key_non_confirm(SYMBOL_PATH).data
    public_key = client.parse_get_public_key_response(response)
    check_get_public_key_resp(backend, public_key)


def test_get_public_key_confirm_accepted(scenario_navigator: NavigateWithScenario):
    client = SymbolClient(scenario_navigator.backend)
    with client.send_async_get_public_key_confirm(SYMBOL_PATH):
        scenario_navigator.address_review_approve(ROOT_SCREENSHOT_PATH)
    response = client.get_async_response()
    assert response is not None
    public_key = client.parse_get_public_key_response(response.data)
    check_get_public_key_resp(scenario_navigator.backend, public_key)


# In this test we check that the GET_PUBLIC_KEY in confirmation mode replies an error if the user refuses
def test_get_public_key_confirm_refused(scenario_navigator: NavigateWithScenario):
    client = SymbolClient(scenario_navigator.backend)

    try:
        with client.send_async_get_public_key_confirm(SYMBOL_PATH):
            scenario_navigator.address_review_reject(ROOT_SCREENSHOT_PATH)
    except ExceptionRAPDU as e:
        assert e.status == ErrorType.ADDRESS_REJECTED
