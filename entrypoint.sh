#!/usr/bin/env bash

set -e

cat <<EOF

====================
== NVIDIA HPC SDK ==
====================
 
NVIDIA HPC SDK version ${HPCSDK_VERSION}
 
Copyright (c) ${HPCSDK_RELEASE}, NVIDIA CORPORATION & AFFILIATES.  All rights reserved.
EOF
 
if [[ "$(find /usr /.singularity.d -name libcuda.so.1 2>/dev/null) " == " " || "$(ls /dev/nvidiactl 2>/dev/null) " == " " ]]; then
  echo
  echo "WARNING: The NVIDIA Driver was not detected.  GPU functionality will not be available."
  if [[ -d /.singularity.d ]]; then
    echo "Use 'singularity run --nv' to start this container; see"
    echo "https://sylabs.io/guides/3.5/user-guide/gpu.html"
  else
    echo "Use 'docker run --gpus all' to start this container; see"
    echo "https://github.com/NVIDIA/nvidia-docker/wiki/Installation-(Native-GPU-Support)"
  fi
fi

echo
 
if [[ $# -eq 0 ]]; then
  exec "/bin/bash"
else
  exec "$@"
fi
