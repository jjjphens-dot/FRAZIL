"""Compatibility entry: execute every extracted validation path."""
import b1_cli_smoke
import b1_cli_contract
import b1_cli_full_matrix
import b1_listening_pack_validation

from cli_support import run_main

def main():
    b1_cli_smoke.main()
    b1_cli_contract.main()
    b1_cli_full_matrix.main()
    b1_listening_pack_validation.main()

if __name__ == "__main__":
    run_main(main)
