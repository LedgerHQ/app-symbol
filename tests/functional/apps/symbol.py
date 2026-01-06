from contextlib import contextmanager
from enum import IntEnum
from typing import Generator, Optional
from struct import pack

from ragger.backend.interface import BackendInterface, RAPDU
from ragger.utils import split_message
from ragger.bip import pack_derivation_path


TESTNET = 152
MAINNET = 104
MIJIN_MAINNET = 96
MIJIN_TESTNET = 144


class INS(IntEnum):
    INS_GET_PUBLIC_KEY = 0x02
    INS_SIGN = 0x04
    INS_GET_VERSION = 0x06


CLA = 0xE0

P1_CONFIRM = 0x01
P1_NON_CONFIRM = 0x00
P2_NO_CHAINCODE = 0x00
P2_CHAINCODE = 0x01
P1_MASK_ORDER = 0x01
P1_MASK_MORE = 0x80
P2_SECP256K1 = 0x40
P2_ED25519 = 0x80

STATUS_OK = 0x9000

MAX_CHUNK_SIZE = 255


class ErrorType:
    NO_APDU_RECEIVED = 0x6982
    UNKNOWN_INSTRUCTION_CLASS = 0x6E00
    UNKNOWN_INSTRUCTION = 0x6D00
    WRONG_APDU_DATA_LENGTH = 0x6A87

    INVALID_PKG_KEY_LENGTH = 0x6A80
    INVALID_BIP32_PATH_LENGTH = 0x6A81
    INVALID_P1_OR_P2 = 0x6B00
    WRONG_RESPONSE_LENGTH = 0xB000

    ADDRESS_REJECTED = 0x6985
    TRANSACTION_REJECTED = 0x6986

    INVALID_SIGNING_PACKET_ORDER = 0x6A82
    SIGNING_DATA_TOO_LARGE = 0x6700
    TOO_MANY_TRANSACTION_FIELDS = 0x6701
    INVALID_TRANSACTION_DATA = 0x6702
    INVALID_INTERNAL_SIGNING_STATE = 0x6703
    INVALID_SIGNING_DATA = 0x6A82

    INTERNAL_ERROR = 0x6A83


class SymbolClient:
    def __init__(self, backend: BackendInterface):
        self._backend = backend

    def send_get_version(self) -> tuple[int, int, int]:
        rapdu: RAPDU = self._backend.exchange(CLA, INS.INS_GET_VERSION, 0, 0, b"")
        response = rapdu.data
        # response = 0x00 (1) ||
        #            MAJOR_VERSION (1) ||
        #            MINOR_VERSION (1) ||
        #            PATCH_VERSION (1)
        assert len(response) == 4
        assert int(response[0]) == 0
        major = int(response[1])
        minor = int(response[2])
        patch = int(response[3])
        return (major, minor, patch)

    def parse_get_public_key_response(self, response: bytes) -> bytes:
        # response = public_key_len (1) ||
        #            public_key (32)
        assert len(response) == 1 + 32
        assert response[0] == 32
        public_key: bytes = response[1: 1 + 32]

        return public_key

    def send_get_public_key_non_confirm(self, derivation_path: str,
                                        network_type: int = MAINNET) -> RAPDU:
        p1 = P1_NON_CONFIRM
        p2 = P2_ED25519
        payload = pack_derivation_path(derivation_path) + pack("<B", network_type)
        return self._backend.exchange(CLA, INS.INS_GET_PUBLIC_KEY,
                                      p1, p2, payload)

    @contextmanager
    def send_async_get_public_key_confirm(self, derivation_path: str,
                                          network_type: int = MAINNET) -> Generator[None, None, None]:
        p1 = P1_CONFIRM
        p2 = P2_ED25519
        payload = pack_derivation_path(derivation_path) + pack("<B", network_type)
        with self._backend.exchange_async(CLA, INS.INS_GET_PUBLIC_KEY,
                                          p1, p2, payload):
            yield

    def _send_sign_message(self, message: bytes, first: bool, last: bool) -> RAPDU:
        p1 = 0
        if not first:
            p1 |= P1_MASK_ORDER
        if not last:
            p1 |= P1_MASK_MORE
        p2 = P2_ED25519
        return self._backend.exchange(CLA, INS.INS_SIGN, p1, p2, message)

    @contextmanager
    def _send_async_sign_message(self, message: bytes,
                                 first: bool, last: bool) -> Generator[None, None, None]:
        p1 = 0
        if not first:
            p1 |= P1_MASK_ORDER
        if not last:
            p1 |= P1_MASK_MORE
        p2 = P2_ED25519
        with self._backend.exchange_async(CLA, INS.INS_SIGN, p1, p2, message):
            yield

    @contextmanager
    def send_async_sign_message(self,
                                derivation_path: str,
                                message: bytes) -> Generator[None, None, None]:
        messages = split_message(pack_derivation_path(derivation_path) + message, MAX_CHUNK_SIZE)
        first = True

        if len(messages) > 1:
            self._send_sign_message(messages[0], True, False)
            for m in messages[1:-1]:
                self._send_sign_message(m, False, False)
            first = False

        with self._send_async_sign_message(messages[-1], first, True):
            yield

    def get_async_response(self) -> Optional[RAPDU]:
        return self._backend.last_async_response
