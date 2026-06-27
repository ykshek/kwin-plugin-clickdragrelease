default: build
build:
    #!/usr/bin/env bash
    set -euo pipefail
    rm -rf build/
    cmake -B build -S .   -DCMAKE_BUILD_TYPE=Release   -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build --parallel $(nproc)
    echo "Done!"
rpm: build
    #!/usr/bin/env bash
    set -euo pipefail
    cd build
    cpack -G RPM
    echo "Done!"
