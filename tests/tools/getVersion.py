#!/usr/bin/env python3

import sys

from pathlib import Path

from ragger.backend import LedgerCommBackend

SYMBOL_LIB_DIRECTORY = (Path(__file__).resolve().parent.parent / "functional").resolve().as_posix()
sys.path.append(SYMBOL_LIB_DIRECTORY)
from apps.symbol import SymbolClient


def main():
    with LedgerCommBackend(None, interface="hid") as backend:
        zilliqa = SymbolClient(backend)
        version = zilliqa.send_get_version()
        print("v{}.{}.{}".format(version[0], version[1], version[2]))


if __name__ == "__main__":
    main()
