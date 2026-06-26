image_name := "localhost/kwin-rmb-plugin:dev"
default: build

build-container:
    #!/usr/bin/env bash
    echo "Building development container image..."
    podman build -t {{image_name}} -f Containerfile .

build: build-container
    #!/usr/bin/env bash
    set -euo pipefail
    rm -rf build/
    mkdir -p build
    podman run --rm \
        --userns=keep-id \
        --volume "$(pwd):/workspace:Z" \
        --workdir /workspace \
        {{image_name}} \
        bash -c '
            set -euo pipefail
            cmake -B build -S .
            cmake --build build --parallel $(nproc)
        '
    echo "Done!"
rpm: build
    #!/usr/bin/env bash
    set -euo pipefail
    podman run --rm \
        --userns=keep-id \
        --volume "$(pwd):/workspace:Z" \
        --workdir /workspace \
        {{image_name}} \
        bash -c "
            set -euo pipefail
            cd build
            cpack -G RPM
        "
    echo "RPM build complete!"
    echo "Built packages:"
    find ./ -name "*.rpm" -type f
