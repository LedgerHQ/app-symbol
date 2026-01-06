#!/usr/bin/env python3

import sys

from pathlib import Path

from ragger.backend import LedgerCommBackend

SYMBOL_LIB_DIRECTORY = (Path(__file__).resolve().parent.parent / "functional").resolve().as_posix()
sys.path.append(SYMBOL_LIB_DIRECTORY)
# pylint: disable=wrong-import-position
from apps.symbol import SymbolClient
# pylint: enable=wrong-import-position


def main():
    with LedgerCommBackend(None, interface="hid") as backend:
        zilliqa = SymbolClient(backend)
        version = zilliqa.send_get_version()
        print(f"v{version[0]}.{version[1]}.{version[2]}")


if __name__ == "__main__":
    main()
