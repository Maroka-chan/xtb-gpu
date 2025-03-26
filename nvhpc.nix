{
  stdenvNoCC,
  fetchzip,
  flock,
  libgcc,
  libz,
  zstd,
  libxml2,
  glibc,
  bc,
  file,
  gcc10
}:
stdenvNoCC.mkDerivation rec {
  name = "nvhpc";
  version = "21.1";
  src = fetchzip {
    url = "https://developer.download.nvidia.com/hpc-sdk/21.1/nvhpc_2021_211_Linux_x86_64_cuda_11.2.tar.gz";
    hash = "sha256-e1h8vyb7grhIEWBUfFQcsfMSHlqszgBWJQq14pr4rvY=";
  };

  dontConfigure = true;
  dontBuild = true;
  dontFixup = true;

  nativeBuildInputs = [
    flock
    gcc10
    file
    bc
  ];

  patchPhase = ''
    find ./install_components/Linux_x86_64/${version} -type f -executable -exec file {} + | awk -F: '/dynamically linked/ && !/shared object/ {print $1}' \
      | xargs -I{} patchelf --set-interpreter "$(cat "$NIX_CC"/nix-support/dynamic-linker)" --set-rpath "${gcc10.cc.lib}/lib:${libgcc.out}/lib:${libz}/lib:${zstd.out}/lib:${libxml2}/lib" "{}"
  '';

  installPhase = ''
    mkdir -p $out

    sed -i 's|/bin/chmod|chmod|g' install_components/install
    sed -i 's|/sbin/ldconfig|ldconfig|g' install_components/install
    patchShebangs install_components/Linux_x86_64/${version}/compilers/bin/makelocalrc
    sed -i '/makelocalrc executed by/d' install_components/Linux_x86_64/${version}/compilers/bin/makelocalrc

    patchShebangs ./install
    NVHPC_SILENT=true NVHPC_INSTALL_DIR=$out NVHPC_INSTALL_TYPE=single ./install

    # fix /usr/lib/crt1.o impure path used in link
    cat >> $out/Linux_x86_64/${version}/compilers/bin/localrc << EOF

    set DEFLIBDIR=${glibc}/lib;
    set DEFSTDOBJDIR=${glibc}/lib;
    EOF
  '';
}
