{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-24.11";
    glibc.url = "github:nixos/nixpkgs/3b05df1d13c1b315cecc610a2f3180f6669442f0";
  };

  outputs = inputs @ { self, nixpkgs, ... }: let
    pkgs_new = import nixpkgs { system = "x86_64-linux"; config.allowUnfree = true; };
    pkgs = import inputs.glibc { system = "x86_64-linux"; config.allowUnfree = true; };

    gfortran = pkgs.buildPackages.wrapCC (pkgs.buildPackages.gcc10.cc.override {
          name = "gfortran";
          langFortran = true;
          langCC = true;
          langC = true;
          profiledCompiler = false;
      });
  in {
    packages.x86_64-linux.default = pkgs.callPackage ./nvhpc.nix { inherit gfortran; libz = pkgs_new.libz; };#{ glibc = pkgs2.glibc; libgcc = pkgs2.gcc10.cc.lib; gcc10 = pkgs2.gcc10; };
    packages.x86_64-linux.nvhpcStdenv = pkgs.callPackage ./nvhpcStdenv.nix {};
    devShells.x86_64-linux.default = pkgs.stdenvNoCC.mkDerivation {
      name = "shell";
      nativeBuildInputs = with pkgs; [
        pkg-config
	gfortran
	gfortran.cc
      ];
      buildInputs = with pkgs; [
        meson
        ninja
        lapack
        lapack.dev
        openblasCompat
        openblasCompat.dev
        python3
        pkgs_new.mkl
        cmake
        asciidoctor
      ];

      shellHook = let
        nvhome = self.packages.x86_64-linux.default;
        target = "Linux_x86_64";
        version = "21.1";
        nvcudadir = "${nvhome}/${target}/${version}/cuda";
        nvcompdir = "${nvhome}/${target}/${version}/compilers";
        nvmathdir = "${nvhome}/${target}/${version}/math_libs";
        nvcommdir = "${nvhome}/${target}/${version}/comm_libs";
      in ''
        export NVHPC=${nvhome}
        export CC=${nvcompdir}/bin/nvc
        export CXX=${nvcompdir}/bin/nvc++
        export FC=${nvcompdir}/bin/nvfortran
        export F90=${nvcompdir}/bin/nvfortran
        export F77=${nvcompdir}/bin/nvfortran
        export CPP=cpp

        export PATH=${nvcommdir}/mpi/bin:${nvcompdir}/bin:${nvcudadir}/bin:$PATH
        export LD_LIBRARY_PATH=${nvcommdir}/nvshmem/lib:${nvcommdir}/nccl/lib:${nvcommdir}/mpi/lib:${nvmathdir}/lib64:${nvcompdir}/lib:${nvcudadir}/extras/CUPTI/lib64:${nvcudadir}/lib64:$LD_LIBRARY_PATH
        export CPATH=${nvcommdir}/nvshmem/include:${nvcommdir}/nccl/include:${nvcommdir}/mpi/include:${nvmathdir}/include:$CPATH
        export MANPATH=${nvcompdir}/man:$MANPATH
        export OPAL_PREFIX=${nvcommdir}/mpi
      '';
    };
  };
}
