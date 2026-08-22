# Packaging

MediaFlyouts ships three release artifacts, all produced by
[`.github/workflows/release.yml`](../.github/workflows/release.yml) when a
`v*.*.*` tag is pushed:

| Artifact | File | Built from |
|----------|------|------------|
| Installer | `MediaFlyouts-<ver>-setup.exe` | [`MediaFlyouts.iss`](MediaFlyouts.iss) (Inno Setup) |
| MSIX package | `MediaFlyouts-<ver>.msix` | [`msix/AppxManifest.xml`](msix/AppxManifest.xml) |
| Source ZIP | `MediaFlyouts-<ver>-src.zip` | `git archive` of the tag |

A portable ZIP (`MediaFlyouts-<ver>-win64.zip`) of the deployed `bin/` folder is
produced as well.

All artifacts are built from the **release** preset with the Qt runtime staged
next to the executable by `windeployqt`.

## Build locally

```powershell
# 1. build + deploy the app into bin\
cmake --preset msvc-release -D "CMAKE_PREFIX_PATH=<Qt>/msvc2022_64"
cmake --build --preset msvc-release
& "<Qt>/msvc2022_64/bin/windeployqt.exe" --release --qmldir Views bin\MediaFlyouts.exe

# 2. installer (needs Inno Setup 6; version is read from the VERSION file)
& "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" packaging\MediaFlyouts.iss

# 3. source zip
git archive --format=zip -o dist\MediaFlyouts-1.0.0-src.zip HEAD

# 4. MSIX (needs the Windows SDK: makeappx + signtool)
#    Stage bin\ + the manifest + Images\ into a layout folder, then:
makeappx pack /d <layout> /p dist\MediaFlyouts-1.0.0.msix
signtool sign /fd SHA256 /a /f <cert.pfx> /p <password> dist\MediaFlyouts-1.0.0.msix
```

The version comes from the top-level [`VERSION.txt`](../VERSION.txt) file, which
is the single source of truth for the app, the installer and the release
workflow.

## Icon assets

The app icon (`packaging/assets/MediaFlyouts.ico`, embedded into the exe via
[`MediaFlyouts.rc`](MediaFlyouts.rc)) and the MSIX tile logos
(`packaging/msix/Images/`) are generated from code by
[`tools/generate_icon.ps1`](../tools/generate_icon.ps1). Re-run it to regenerate
them after changing the design:

```powershell
powershell -ExecutionPolicy Bypass -File tools\generate_icon.ps1
```

## Secrets you must provide before the first release

The only thing not in the repo is the **code-signing certificate** for the MSIX.
The `Publisher` in `AppxManifest.xml` must equal the certificate subject
(`CN=MediaFlyouts`).

[`tools/new-signing-cert.ps1`](../tools/new-signing-cert.ps1) creates a matching
self-signed certificate, signs a package, and can export the PFX + base64 for CI:

```powershell
# sign the local package (self-signed, for testing)
.\tools\new-signing-cert.ps1 -Msix release\MediaFlyouts-1.0.0.msix

# to install a self-signed package, trust its cert once as a root (run elevated):
Import-Certificate -FilePath signing\MediaFlyouts.cer -CertStoreLocation Cert:\LocalMachine\Root

# export a PFX + base64 for CI secrets
.\tools\new-signing-cert.ps1 -ExportPfx -PfxPassword (Read-Host 'PFX password' -AsSecureString)
```

Set these repository secrets so the release workflow signs the MSIX:

- `MSIX_CERT_BASE64` — base64 of the `.pfx`
- `MSIX_CERT_PASSWORD` — the `.pfx` password

For a real (non-self-signed) release, use a certificate from a trusted CA so
users don't have to trust it manually. The installer and ZIP artifacts do not
require any signing.
