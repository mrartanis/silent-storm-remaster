#!/usr/bin/env bash
set -euo pipefail

unsigned=$1
output=$(realpath -m "$2")
build_tools="$ANDROID_HOME/build-tools/36.0.0"
signing=$(mktemp -d)
trap 'rm -rf "$signing"' EXIT
umask 077

alias=silent-storm-android
if [[ ${ANDROID_REQUIRE_RELEASE_KEY:-false} == true ]]; then
  : "${ANDROID_RELEASE_KEYSTORE:?Android release keystore secret is missing}"
  : "${ANDROID_RELEASE_KEYSTORE_PASSWORD:?Android release keystore password secret is missing}"
  printf '%s' "$ANDROID_RELEASE_KEYSTORE" | base64 --decode > "$signing/key.p12"
  printf '%s\n' "$ANDROID_RELEASE_KEYSTORE_PASSWORD" > "$signing/password"
else
  openssl rand -hex 32 > "$signing/password"
  keytool -genkeypair -keystore "$signing/key.p12" -storetype PKCS12 \
    -storepass:file "$signing/password" -alias "$alias" -keyalg RSA -keysize 2048 \
    -validity 30 -dname 'CN=Silent Storm CI test'
fi

mkdir -p "$(dirname "$output")"
"$build_tools/zipalign" -P 16 -f 4 "$unsigned" "$signing/aligned.apk"
"$build_tools/apksigner" sign --ks "$signing/key.p12" --ks-pass "file:$signing/password" \
  --ks-key-alias "$alias" --v4-signing-enabled false --out "$output" "$signing/aligned.apk"
"$build_tools/apksigner" verify --verbose --print-certs "$output"
"$build_tools/zipalign" -c -P 16 4 "$output"
(cd "$(dirname "$output")" && sha256sum "$(basename "$output")" > "$(basename "$output").sha256")
