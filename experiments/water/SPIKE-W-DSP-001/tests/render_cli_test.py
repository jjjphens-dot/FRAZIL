"""Compatibility entry: execute every extracted validation path."""
import render_cli_smoke
import render_cli_contract
import render_cli_full_matrix

from cli_support import run_main

def main():
    render_cli_smoke.main()
    render_cli_contract.main()
    render_cli_full_matrix.main()

if __name__ == "__main__":
    run_main(main)
