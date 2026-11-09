from json import load
from os import listdir
from pathlib import Path

from apps.symbol_transaction_builder import encode_txn_context

ROOT_SCREENSHOT_PATH = Path(__file__).parent.resolve()

CORPUS_DIR = Path(__file__).parent.parent / "corpus"
CORPUS_FILES = listdir(CORPUS_DIR)

# Proposed XYM derivation paths for tests ###
SYMBOL_PATH = "m/44'/4343'/0'/0'/0'"


def load_transaction_from_file(transaction_filename: str) -> tuple[bytes,str]:
    with open(CORPUS_DIR / transaction_filename, encoding="utf-8") as f:
        transaction = load(f)
    return encode_txn_context(transaction), transaction['common_txn_header']['transactionType']
