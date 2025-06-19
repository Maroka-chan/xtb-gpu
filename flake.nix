{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    nixpkgs_old.url = "github:nixos/nixpkgs/3b05df1d13c1b315cecc610a2f3180f6669442f0";
  };

  outputs = inputs @ { self, nixpkgs, ... }: let
    pkgs = import nixpkgs { system = "x86_64-linux"; config.allowUnfree = true; };
    pkgs_old = import inputs.nixpkgs_old { system = "x86_64-linux"; config.allowUnfree = true; };
  in {
    packages.x86_64-linux = rec {
      default = xtb;
      xtb = pkgs_old.callPackage ./xtb.nix { nvfortran = self.packages.x86_64-linux.nvhpc; mkl = pkgs.mkl; };
      nvhpc = pkgs_old.callPackage ./nvhpc.nix { libz = pkgs.libz; };
      "dpc++" = pkgs.callPackage ./dpc++.nix {};
      #ifort = pkgs.callPackage ./intel-ifort.nix {};
    };

    devShells.x86_64-linux.xtb = pkgs.mkShell {
      packages = with pkgs; [
        meson
        ninja
        gfortran
        pkg-config
        cmake
        lapack
        python3
        blas
        asciidoctor
        tblite
        go
        toml-f
        test-drive
        simple-dftd3
        mstore
        multicharge
        dftd4
        json-fortran
        mctc-lib
      ];

      shellHook = ''
        export FC=gfortran CC=gcc
      '';
    };

    devShells.x86_64-linux.default = pkgs.mkShell {
      packages = with pkgs; [
        adaptivecppWithRocm
      ];
    };
  };
}
