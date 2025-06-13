{
  stdenvNoCC,
  fetchFromGitHub,
  gitMinimal,
  cmake,
  python3,
  ninja,
  hwloc,
  clang,
  level-zero,
  numactl,
  opencl-headers,
  khronos-ocl-icd-loader
}:
stdenvNoCC.mkDerivation {
  name = "dpc++";
  version = "6.0.1";

  srcs = [
    (fetchFromGitHub {
      owner = "intel";
      repo = "llvm";
      rev = "180f73e6cd32c9fcb6ca9d9700545b80ed4c469c";
      sha256 = "sha256-GuAcsMSbNPtC/ShDOk6yNKY014nmjIDnTd/PszJASUM=";
    })

    (fetchFromGitHub {
      owner = "intel";
      repo = "vc-intrinsics";
      rev = "4e51b2467104a257c22788e343dafbdde72e28bb";
      sha256 = "sha256-AHXeKbih4bzmcuu+tx2TeS7Ixmk54uS1vKFVxI6ZP8g=";
    })

    (fetchFromGitHub {
      owner = "oneapi-src";
      repo = "unified-runtime";
      rev = "d03f19a88e42cb98be9604ff24b61190d1e48727";
      sha256 = "sha256-gbOWgwFDbT/y8cXZHv6yEAP2EZvTxNeKmcY3OQsnXGA=";
    })

    (fetchFromGitHub {
      owner = "intel";
      repo = "compute-runtime";
      rev = "25.05.32567.17";
      sha256 = "sha256-/9UQJ5Ng2ip+3cNcVZOtKAmnx4LpmPja+aTghIqF1bc=";
    })

    (fetchFromGitHub {
      owner = "oneapi-src";
      repo = "unified-memory-framework";
      rev = "v0.11.0-rc1";
      sha256 = "sha256-sR7Gkxl9jrtHGBls+9qkRt/rxFIvJPRUCkh30vEXjnk=";
    })

    (fetchFromGitHub {
      owner = "google";
      repo = "googletest";
      rev = "release-1.12.1";
      sha256 = "sha256-W+OxRTVtemt2esw4P7IyGWXOonUN5ZuscjvzqkYvZbM=";
    })
  ];

  #dontPatch = true;
  #dontConfigure = true;
  #dontBuild = true;
  dontFixup = true;

  nativeBuildInputs = [
    gitMinimal
    cmake   # >= 3.20.0
    python3 # >= 3.8
    ninja
    hwloc   # >= 2.3
    clang   # >= 5.0
    #level-zero
    numactl.dev
  ];

  unpackPhase = ''
    runHook preUnpack

    sources=($srcs)
    sourceRoot="source"

    mkdir -p "$sourceRoot"

    cp -r ''${sources[0]} "$sourceRoot"/llvm
    cp -r ''${sources[1]} "$sourceRoot"/vc-intrinsics
    cp -r ''${sources[2]} "$sourceRoot"/unified-runtime
    cp -r ''${sources[3]} "$sourceRoot"/compute-runtime
    cp -r ''${sources[4]} "$sourceRoot"/unified-memory-framework
    cp -r ''${sources[5]} "$sourceRoot"/googletest

    chmod -R u+w -- "$sourceRoot"

    runHook postUnpack
  '';

  patchPhase = ''
    runHook prePatch

    patch -i ${./llvm-fetching.patch} -p1 -d llvm
    patch -i ${./umf-fetching.patch} -p1 -d unified-memory-framework

    runHook postPatch
  '';

  configurePhase = ''
    runHook preConfigure

    export DPCPP_HOME="$PWD"/llvm
    python $DPCPP_HOME/buildbot/configure.py \
      -o $DPCPP_HOME/build \
      -t Release \
      -DLLVMGenXIntrinsics_SOURCE_DIR=$PWD/vc-intrinsics \
      -DSYCL_UR_USE_FETCH_CONTENT=OFF \
      -DSYCL_UR_SOURCE_DIR=$PWD/unified-runtime \
      -DUR_LEVEL_ZERO_LOADER_LIBRARY=${level-zero}/lib \
      -DUR_LEVEL_ZERO_INCLUDE_DIR=${level-zero}/include \
      -DCOMPUTE_RUNTIME_LEVEL_ZERO_INCLUDE=$PWD/compute-runtime/level_zero/include \
      -DUMF_SOURCE_DIR=$PWD/unified-memory-framework \
      -DGTEST_SOURCE_DIR=$PWD/googletest \
      -DOpenCL_HEADERS=${opencl-headers} \
      -DOpenCL_LIBRARY_SRC=${khronos-ocl-icd-loader}

    runHook postConfigure
  '';

  buildPhase = ''
    runHook preBuild

    python $DPCPP_HOME/buildbot/compile.py \
      -o $DPCPP_HOME/build

    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall

    mkdir -p $out

    runHook postInstall
  '';
}
