#!/usr/bin/env bash
set -euo pipefail

VERSION="${1:-1.0.0}"
mkdir -p "${2:-artifacts}"
OUTPUT_DIR="$(cd "${2:-artifacts}" && pwd)"
BUILD_DIR="$(cd "${3:-build}" && pwd)"

PLUGIN_BUNDLE="${BUILD_DIR}/motionfx.plugin"

# Ensure bundle structure
mkdir -p "${PLUGIN_BUNDLE}/Contents/MacOS"
mkdir -p "${PLUGIN_BUNDLE}/Contents/Resources/locale"

# Copy binary if not already placed
if [[ -f "${BUILD_DIR}/motionfx.so" ]]; then
    cp "${BUILD_DIR}/motionfx.so" "${PLUGIN_BUNDLE}/Contents/MacOS/motionfx"
elif [[ -f "${BUILD_DIR}/libmotionfx.dylib" ]]; then
    cp "${BUILD_DIR}/libmotionfx.dylib" "${PLUGIN_BUNDLE}/Contents/MacOS/motionfx"
fi

# Copy Info.plist
cp installer/macos/Info.plist "${PLUGIN_BUNDLE}/Contents/Info.plist"

# Copy Resources (Shaders & Locales)
cp data/motionfx_3d.effect "${PLUGIN_BUNDLE}/Contents/Resources/"
cp data/locale/*.ini "${PLUGIN_BUNDLE}/Contents/Resources/locale/"

# 1. Generate ZIP
echo "Creating macOS ZIP package..."
(cd "${BUILD_DIR}" && zip -r -y "../../${OUTPUT_DIR}/MotionFX-v${VERSION}-macOS.zip" motionfx.plugin) || \
(cd "$(dirname "${PLUGIN_BUNDLE}")" && zip -r -y "${OUTPUT_DIR}/MotionFX-v${VERSION}-macOS.zip" motionfx.plugin)

# 2. Generate PKG
echo "Building macOS Installer PKG..."
PKG_ROOT="${BUILD_DIR}/pkg_root"
rm -rf "${PKG_ROOT}"
mkdir -p "${PKG_ROOT}"
cp -R "${PLUGIN_BUNDLE}" "${PKG_ROOT}/"

pkgbuild --root "${PKG_ROOT}" \
         --identifier "com.kubolive.motionfx" \
         --version "${VERSION}" \
         --install-location "/Library/Application Support/obs-studio/plugins" \
         "${OUTPUT_DIR}/MotionFX-v${VERSION}-macOS.pkg"

echo "macOS packaging completed successfully:"
ls -lh "${OUTPUT_DIR}"
