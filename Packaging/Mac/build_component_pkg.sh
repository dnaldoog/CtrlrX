#!/bin/bash
# Packaging/Mac/build_component_pkg.sh
#
# Builds a component .pkg for one bundle (.app, .vst3 or .component) so that the installer
#   - installs to the stated location (BundleIsRelocatable = false), and
#   - replaces whatever is installed, even if it believes the installed version is newer
#     (BundleIsVersionChecked = false). This is needed because "8.79" compares as NEWER than
#     "8.0.79", so the default version check silently skips the upgrade.
#
# Usage:
#   build_component_pkg.sh <bundle> <package-identifier> <version> <install-location> <output.pkg>
#
# Example:
#   build_component_pkg.sh "$VST3_PATH" "$BUNDLE_ID.vst3.pkg" "$VERSION" \
#       "/Library/Audio/Plug-Ins/VST3" "Packaging/CtrlrX.vst3.pkg"

set -euo pipefail

if [ "$#" -ne 5 ]; then
	echo "usage: $0 <bundle> <package-identifier> <version> <install-location> <output.pkg>" >&2
	exit 2
fi

SRC="$1"
IDENTIFIER="$2"
VERSION="$3"
LOCATION="$4"
OUT="$5"

if [ ! -e "$SRC" ]; then
	echo "Bundle not found: $SRC" >&2
	exit 1
fi

ROOT="$(mktemp -d)"
PLIST="$(mktemp -t components).plist"
trap 'rm -rf "$ROOT" "$PLIST"' EXIT

# ditto (unlike cp -R) keeps extended attributes and resource forks, which hold the custom bundle icon
ditto "$SRC" "$ROOT/$(basename "$SRC")"

# Generate the default component plist: an array with one dictionary per bundle found in the payload
pkgbuild --analyze --root "$ROOT" "$PLIST"

# Edit every entry. (The top level of this plist is an array, so each key needs an index.)
COUNT=0
while /usr/libexec/PlistBuddy -c "Print :$COUNT:BundleIsRelocatable" "$PLIST" >/dev/null 2>&1; do
	/usr/libexec/PlistBuddy -c "Set :$COUNT:BundleIsRelocatable false" "$PLIST"
	/usr/libexec/PlistBuddy -c "Set :$COUNT:BundleIsVersionChecked false" "$PLIST"
	COUNT=$((COUNT + 1))
done

if [ "$COUNT" -eq 0 ]; then
	echo "pkgbuild --analyze found no bundles in $SRC" >&2
	exit 1
fi

echo "Component plist ($COUNT bundle entries):"
/usr/libexec/PlistBuddy -c "Print" "$PLIST"

pkgbuild \
	--root "$ROOT" \
	--component-plist "$PLIST" \
	--identifier "$IDENTIFIER" \
	--version "$VERSION" \
	--install-location "$LOCATION" \
	"$OUT"
