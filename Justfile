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

    # Create RPM build structure
    mkdir -p rpmbuild/{SOURCES,SPECS,BUILD,RPMS,SRPMS}

    # Extract version from kwin_contextmenudrag.json or fallback to 0.1.0
    VERSION=$(grep '"Version":' kwin_contextmenudrag.json | sed -E 's/.*"Version": "([^"]+)".*/\1/' || echo "1.0.0")

    # Create source tarball matching %autosetup layout
    echo "Creating source tarball v${VERSION}..."
    tar --exclude='./rpmbuild' --exclude='./build' --exclude='./.git' \
        -czf rpmbuild/SOURCES/v${VERSION}.tar.gz \
        --transform "s|^\.|v${VERSION}|" .

    # Copy spec file
    cp kwin-plugin-clickdragrelease.spec rpmbuild/SPECS/

    # Build RPM using development container
    podman run --rm \
        --userns=keep-id \
        --volume "$(pwd):/workspace:Z" \
        --workdir /workspace \
        {{image_name}} \
        bash -c "
            set -euo pipefail

            # Build the RPM (binary and source)
            rpmbuild --define '_topdir /workspace/rpmbuild' \
                     --define 'version ${VERSION}' \
                     -ba rpmbuild/SPECS/kwin-plugin-clickdragrelease.spec
        "

    echo "RPM build complete!"
    echo "Built packages:"
    find rpmbuild/RPMS -name "*.rpm" -type f
    find rpmbuild/SRPMS -name "*.rpm" -type f

bump-version version:
    #!/usr/bin/env bash
    set -euo pipefail

    VERSION="{{version}}"

    # Validate version format (should be like 1.2.3)
    if ! echo "$VERSION" | grep -qE '^[0-9]+\.[0-9]+\.[0-9]+$'; then
        echo "Error: Version must be in format X.Y.Z (e.g., 1.2.3)"
        exit 1
    fi

    echo "Bumping version to $VERSION..."

    # Check working directory is clean
    if ! git diff --quiet || ! git diff --cached --quiet; then
        echo "Error: Working directory is not clean. Please commit or stash changes first."
        git status --porcelain
        exit 1
    fi

    # Update version in kwin_contextmenudrag.json
    echo "Updating version in kwin_contextmenudrag.json..."
    sed -i "s/\"Version\": \"[^\"]*\"/\"Version\": \"$VERSION\"/" kwin_contextmenudrag.json

    # Update version in spec file
    echo "Updating version in kwin-plugin-clickdragrelease.spec..."
    sed -i "s/^Version:.*/Version:        $VERSION/" kwin-plugin-clickdragrelease.spec

    # Verify changes
    if ! grep -q "\"Version\": \"$VERSION\"" kwin_contextmenudrag.json; then
        echo "Error: Failed to update version in kwin_contextmenudrag.json"
        exit 1
    fi
    if ! grep -q "^Version:[[:space:]]*$VERSION" kwin-plugin-clickdragrelease.spec; then
        echo "Error: Failed to update Version in kwin-plugin-clickdragrelease.spec"
        exit 1
    fi

    echo "Version updated successfully in CMakeLists.txt and spec file."
    git diff kwin_contextmenudrag.json kwin-plugin-clickdragrelease.spec
    git add kwin_contextmenudrag.json kwin-plugin-clickdragrelease.spec
    git commit -m "chore: bump version to v$VERSION"

commit version:
    #!/usr/bin/env bash
    set -euo pipefail

    VERSION="{{version}}"

    # Create tag for release and push both commit and tag
    git tag -a v${VERSION} -m "Release v${VERSION}"
    git push origin HEAD
    git push origin v${VERSION}
