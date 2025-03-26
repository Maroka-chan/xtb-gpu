{
  stdenvNoCC,
  fetchurl,
  autoPatchelfHook,
  flock,
  bc,
  gfortran,
  file,
  ncurses,
  kdePackages
}: let
  version = "2020.4.304";
in stdenvNoCC.mkDerivation {
  name = "intel-compilers";
  inherit version;
  src = fetchurl {
    url = "https://registrationcenter-download.intel.com/akdlm/IRC_NAS/577ebc28-d0f6-492b-9a43-b04354ce99da/intel-fortran-compiler-2025.1.0.601_offline.sh";
    hash = "sha256-JwFjKd7eg2lnnyK06fZ5NoN855cqHvT1x27ofXyWPIE=";
  };

  dontUnpack = true;
  dontConfigure = true;
  #dontPatch = true;
  dontBuild = true;
  dontFixup = true;

  #dontPatchELF = true;
  #dontStrip = true;

  #enableParallelBuilding = true;

  nativeBuildInputs = [
    autoPatchelfHook
    flock
    gfortran
    gfortran.cc
    file
    ncurses
  ];

  buildInputs = [
    bc
    kdePackages.full
  ];

  patchPhase = ''
    cp $src intel-fortran-compiler.sh
    patchShebangs ./intel-fortran-compiler.sh
    chmod +x ./intel-fortran-compiler.sh
  '';

  installPhase = ''
    mkdir extract
    ./intel-fortran-compiler.sh --silent --extract-only --extract-folder ./extract
    patchelf --set-interpreter "$(cat "$NIX_CC"/nix-support/dynamic-linker)" --set-rpath "${gfortran.cc.lib}/lib:${gfortran.cc.lib}/lib64:${kdePackages.full.out}/lib" ./extract/intel-fortran-compiler-2025.1.0.601_offline/bootstrapper

    patchShebangs ./extract/intel-fortran-compiler-2025.1.0.601_offline/install.sh
    chmod +x ./extract/intel-fortran-compiler-2025.1.0.601_offline/install.sh
    ./extract/intel-fortran-compiler-2025.1.0.601_offline/install.sh
  '';
 #   ./intel-fortran-compiler.sh --silent --extract-only --extract-folder ./extract --remove-extracted-files no -a --eula --install-dir $out
 # '';
}
