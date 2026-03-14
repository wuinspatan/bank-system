# shell.nix – Nix development environment for the Bank Management System
# Usage: nix-shell   (inside bank-system/ directory)

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "bank-system-dev";

  buildInputs = with pkgs; [
    gcc          # C compiler
    gnumake      # make build tool
    gdb          # GNU debugger
    valgrind     # memory error detector
    clang-tools  # clangd LSP + clang-format (optional, for IDE support)
    unzip
  ];

  shellHook = ''
    echo "DeV Shell Already!"
  '';
}
