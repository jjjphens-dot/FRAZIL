"""Compatibility entry: execute every extracted validation path."""
import d1_cli_smoke
import d1_cli_contract
import d1_cli_full_matrix
import d1_native_oracle_validation
import sys

from cli_support import run_main

def main():
    d1_cli_smoke.main()
    d1_cli_contract.main()
    d1_cli_full_matrix.main()
    if len(sys.argv) > 2:
        d1_native_oracle_validation.main()

if __name__ == "__main__":
    run_main(main)
