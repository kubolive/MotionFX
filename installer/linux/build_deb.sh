#!/usr/bin/env bash
set -euo pipefail

VERSION="${1:-1.0.0}"
mkdir -p "${2:-artifacts}"
OUTPUT_DIR="$(cd "${2:-artifacts}" && pwd)"
BUILD_DIR="$(cd "${3:-build}" && pwd)"

PKG_DIR="${BUILD_DIR}/pkg_debian"
rm -rf "${PKG_DIR}"
mkdir -p "${PKG_DIR}/DEBIAN"
mkdir -p "${PKG_DIR}/usr/lib/obs-plugins"
mkdir -p "${PKG_DIR}/usr/share/obs/obs-plugins/motionfx/locale"

# Control file
cat <<EOF > "${PKG_DIR}/DEBIAN/control"
Package: motionfx
Version: ${VERSION}
Section: video
Priority: optional
Architecture: amd64
Maintainer: KuboLive <kubolive@users.noreply.github.com>
Homepage: https://github.com/kubolive/MotionFX
Depends: libobs0 (>= 28.0) | obs-studio (>= 28.0)
Description: MotionFX: Advanced Animation & 3D Transitions for OBS Studio
 MotionFX provides high-performance composable scene item show/hide animations,
 including 3D flips, smooth spring dynamics, text reveals, and camera fly-throughs.
 Disclaimer: MotionFX is an independent third-party open-source project and is
 not affiliated with, sponsored by, or endorsed by the OBS Project.
EOF

# Install plugin binary
if [[ -f "${BUILD_DIR}/motionfx.so" ]]; then
    cp "${BUILD_DIR}/motionfx.so" "${PKG_DIR}/usr/lib/obs-plugins/"
elif [[ -f "${BUILD_DIR}/libmotionfx.so" ]]; then
    cp "${BUILD_DIR}/libmotionfx.so" "${PKG_DIR}/usr/lib/obs-plugins/motionfx.so"
fi

# Install assets
cp data/motionfx_3d.effect "${PKG_DIR}/usr/share/obs/obs-plugins/motionfx/"
cp data/locale/*.ini "${PKG_DIR}/usr/share/obs/obs-plugins/motionfx/locale/"

# Set standard permissions
find "${PKG_DIR}" -type d -exec chmod 755 {} +
find "${PKG_DIR}" -type f -exec chmod 644 {} +
chmod 755 "${PKG_DIR}/usr/lib/obs-plugins/"*.so

# 1. Build .deb
echo "Building .deb package..."
dpkg-deb --build --root-owner-group "${PKG_DIR}" "${OUTPUT_DIR}/MotionFX-v${VERSION}-Linux-x64.deb"

# 2. Build portable tar.gz and zip
echo "Creating portable Linux archive..."
PORTABLE_DIR="${BUILD_DIR}/motionfx-linux-x64"
rm -rf "${PORTABLE_DIR}"
mkdir -p "${PORTABLE_DIR}/obs-plugins/64bit"
mkdir -p "${PORTABLE_DIR}/data/obs-plugins/motionfx/locale"

cp "${PKG_DIR}/usr/lib/obs-plugins/"*.so "${PORTABLE_DIR}/obs-plugins/64bit/"
cp data/motionfx_3d.effect "${PORTABLE_DIR}/data/obs-plugins/motionfx/"
cp data/locale/*.ini "${PORTABLE_DIR}/data/obs-plugins/motionfx/locale/"
cp README.md LICENSE "${PORTABLE_DIR}/" || true

tar -czvf "${OUTPUT_DIR}/MotionFX-v${VERSION}-Linux-x64.tar.gz" -C "${BUILD_DIR}" "motionfx-linux-x64"
(cd "${BUILD_DIR}" && zip -r "${OUTPUT_DIR}/MotionFX-v${VERSION}-Linux-x64.zip" "motionfx-linux-x64")

echo "Linux packaging completed successfully:"
ls -lh "${OUTPUT_DIR}"
