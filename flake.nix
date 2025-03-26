{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-24.11";
    nixpkgs_old.url = "github:nixos/nixpkgs/3b05df1d13c1b315cecc610a2f3180f6669442f0";
  };

  outputs = inputs @ { self, nixpkgs, ... }: let
    pkgs = import nixpkgs { system = "x86_64-linux"; config.allowUnfree = true; };
    pkgs_old = import inputs.nixpkgs_old { system = "x86_64-linux"; config.allowUnfree = true; };

    gfortran = pkgs_old.buildPackages.wrapCC (pkgs_old.buildPackages.gcc10.cc.override {
          name = "gfortran";
          langFortran = true;
          langCC = true;
          langC = true;
          profiledCompiler = false;
      });

    #gfortran_new = pkgs_new.buildPackages.wrapCC (pkgs_new.buildPackages.gcc14.cc.override {
    #      name = "gfortran";
    #      langFortran = true;
    #      langCC = true;
    #      langC = true;
    #      profiledCompiler = false;
    #  });
  in {
    #packages.x86_64-linux.intel-comp = pkgs_new.callPackage ./intel-ifort.nix { gfortran = gfortran_new; kdePackages = pkgs_new.kdePackages; };
    packages.x86_64-linux = rec {
      default = xtb;
      xtb = pkgs_old.callPackage ./xtb.nix { inherit gfortran; nvfortran = self.packages.x86_64-linux.nvhpc; mkl = pkgs.mkl; };
      nvhpc = pkgs_old.callPackage ./nvhpc.nix { inherit gfortran; libz = pkgs.libz; };
    };

    #devShells.x86_64-linux.default = pkgs.stdenvNoCC.mkDerivation {
    #  name = "shell";
    #  nativeBuildInputs = with pkgs; [
    #    pkg-config
    #    gfortran
    #    gfortran.cc
    #  ];
    #  buildInputs = with pkgs; [
    #    meson
    #    ninja
    #    lapack
    #    lapack.dev
    #    openblasCompat
    #    openblasCompat.dev
    #    python3
    #    pkgs_new.mkl
    #    cmake
    #    asciidoctor
    #  ];

    #  shellHook = ''
    #    export FC=ifort
    #    export CC=icc
    #    export PATH=${self.packages.x86_64-linux.intel-comp}/bin
    #  '';
    #};
  };
}
