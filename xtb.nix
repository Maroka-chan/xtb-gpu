{
  stdenvNoCC,
  fetchFromGitHub,
  nvfortran,
  meson,
  ninja,
  mkl,
  lapack,
  openblasCompat,
  cmake,
  asciidoctor,
  pkg-config,
  git,
  hostname,
  gcc10
}:
stdenvNoCC.mkDerivation {
  name = "xtb";
  version = "6.4.0";
  src = fetchFromGitHub {
    owner = "grimme-lab";
    repo = "xtb";
    rev = "v6.4.0";
    sha256 = "sha256-jKmk0lJb1rA9x/ZwMLP/vm0HWd1FaHVAilGN5I1Ljjk=";
  };

  dontConfigure = true;
  dontPatch = true;
  dontFixup = true;

  nativeBuildInputs = [
    pkg-config
    git
    mkl
    gcc10
    meson
    ninja
    cmake
    nvfortran
    lapack.dev
    openblasCompat.dev
    asciidoctor
    hostname
  ];

  buildPhase = let
    nvhome = nvfortran;
    target = "Linux_x86_64";
    version = "21.1";
    nvcudadir = "${nvhome}/${target}/${version}/cuda";
    nvcompdir = "${nvhome}/${target}/${version}/compilers";
    nvmathdir = "${nvhome}/${target}/${version}/math_libs";
    nvcommdir = "${nvhome}/${target}/${version}/comm_libs";
  in ''
    runHook preBuild

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

    meson setup build_gpu \
      -Dopenmp=false \
      -Dla_backend=openblas \
      -Dgpu=true \
      -Dcusolver=false \
      -Dgpu_arch=75 \
      -Dc_std=none \
      -Ddefault_library=static \
      --prefix=$out \
      --bindir=bin \
      --libdir=lib \
      --includedir=include \
      --datadir=share \
      --mandir=man

    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p $out/{bin,lib,include,share,man}
    ninja -C build_gpu install

    runHook postInstall
  '';
}

