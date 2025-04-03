from ragger.backend import SpeculosBackend
from ragger.backend.interface import RaisePolicy
from ragger.navigator import NavInsID
from ragger.error import ExceptionRAPDU

from apps.symbol import SymbolClient, ErrorType
from utils import ROOT_SCREENSHOT_PATH

# Proposed XYM derivation paths for tests ###
SYMBOL_PATH = "m/44'/4343'/0'/0'/0'"

SPECULOS_EXPECTED_PUBLIC_KEY = "73f0bf90d39d1d0a3ec03740eec95c12"\
                               "7a82a20bf07f6840462125a94a42df1e"


def check_get_public_key_resp(backend, public_key):
    if isinstance(backend, SpeculosBackend):
        # Check against nominal Speculos seed expected results
        assert public_key.hex() == SPECULOS_EXPECTED_PUBLIC_KEY


def test_get_public_key_non_confirm(backend):
    client = SymbolClient(backend)
    response = client.send_get_public_key_non_confirm(SYMBOL_PATH).data
    public_key = client.parse_get_public_key_response(response)
    check_get_public_key_resp(backend, public_key)


def test_get_public_key_confirm_accepted(firmware, backend, navigator, test_name, scenario_navigator):
    client = SymbolClient(backend)
    with client.send_async_get_public_key_confirm(SYMBOL_PATH):
        if firmware.device.startswith("nano"):
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Approve",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        else:
            scenario_navigator.address_review_approve(ROOT_SCREENSHOT_PATH, test_name)
    response = client.get_async_response().data
    public_key = client.parse_get_public_key_response(response)
    check_get_public_key_resp(backend, public_key)


# In this test we check that the GET_PUBLIC_KEY in confirmation mode replies an error if the user refuses
def test_get_public_key_confirm_refused(firmware, backend, navigator, test_name, scenario_navigator):
    client = SymbolClient(backend)

    if firmware.device.startswith("nano"):
        with client.send_async_get_public_key_confirm(SYMBOL_PATH):
            backend.raise_policy = RaisePolicy.RAISE_NOTHING
            navigator.navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                                      [NavInsID.BOTH_CLICK],
                                                      "Reject",
                                                      ROOT_SCREENSHOT_PATH,
                                                      test_name)
        rapdu = client.get_async_response()
        assert rapdu.status == ErrorType.ADDRESS_REJECTED
        assert len(rapdu.data) == 0
    else:
        try:
            with client.send_async_get_public_key_confirm(SYMBOL_PATH):
                scenario_navigator.address_review_reject(ROOT_SCREENSHOT_PATH, test_name)
        except ExceptionRAPDU as e:
            assert e.status == ErrorType.ADDRESS_REJECTED
