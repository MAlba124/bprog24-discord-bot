{
  inputs = {
    nixpkgs.url = "nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
          nativeBuildInputs = with pkgs; [
            gcc
            gdb
            gnumake
            binutils
            valgrind
            clang-tools
            gnuplot
          ];
          buildInputs = with pkgs; [
            concord
            curl
          ];
        in
        with pkgs;
        {
          LD_LIBRARY_PATH = lib.makeLibraryPath buildInputs;
          devShells.default = mkShell {
            inherit buildInputs nativeBuildInputs;
          };
        }
      );
}
