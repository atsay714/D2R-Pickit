# Publishing to the D2RLoader Plugin Hub

Maintainer: [atsay714](https://github.com/atsay714).

The workflow in `.github/workflows/publish-hub.yml` adapts the D2RLoader Hub's
release-upload sample. A **published GitHub release** builds and tests the
Windows x64 DLL, verifies its metadata, creates the ZIPs, transfers the build
artifacts to Linux, checks their SHA-256 checksums, and uploads the **native**
global-layout ZIP to the Hub. Release notes become the Hub changelog.

The native package is mod agnostic. The additional Reimagined-layout and
source ZIPs remain available in the workflow's package artifact for 14 days.
The workflow does not attach files to the GitHub release itself.

## One-time setup for this repository

1. Create this plugin's project on the Hub with its first package, following
   the Hub's upload/scan process. Copy the Hub project ID from that project.
   The upload endpoint adds versions to an **existing** project; it does not
   create the initial Hub project. Do not assume the Hub ID equals the runtime
   plugin ID in `plugin.json`.
2. In GitHub **Settings > Secrets and variables > Actions**, add the repository
   secret **D2RLOADER_TOKEN** with your Hub API token.
3. Under **Variables**, add **PLUGIN_ID** with this project's Hub ID.
   **D2RLOADER_PLUGIN_ID** is also accepted for existing configurations.
   Set it separately in each repository so packages go to the correct project.

No token is stored in the workflow or source tree. A release run fails with a
clear message if the token or project ID is missing. The upload retains the
sample's `consent=1` parameter and therefore follows the Hub's upload/scanning
consent behavior.

## Validate before publishing

Open **Actions > Upload to the D2RLoader Plugin Hub > Run workflow** on the main
branch, leaving **release_tag** blank. This builds, tests and packages only,
without running the Hub upload job or requiring Hub credentials. Download the
package artifact from the completed run to inspect its contents.

## Publish a subsequent version

1. Update the version fields listed in [BUILDING.md](BUILDING.md), update the
   changelog, and commit the changes.
2. Create and publish a GitHub release at that source commit, tagged `vX.Y.Z`
   or `X.Y.Z`. The tag, `plugin.json`, and built DLL versions must agree.
3. Enter the release notes; those are sent to the Hub as the changelog.
4. Check the workflow result and the Hub's release/scan status.

Use a version not already uploaded to the Hub. Draft releases do not publish;
publishing a prerelease also triggers the sample's `release: published` event.
Pushing a tag by itself does not upload anything. If configuration was missing,
add it and rerun the failed job. Avoid rerunning an upload that already succeeded:
the POST has no automatic retry, so duplicate-version behavior is left to the Hub.

## Retry after fixing a publishing workflow

If an upload failed before creating the Hub version, run the workflow on
**main** and enter the existing published release tag (for example, `v0.7.0`)
in **release_tag**. This uses the corrected workflow from main while building
the tagged source. It validates that the published release and DLL versions
match, reads that release's changelog, and uploads the native ZIP. It does not
change the Hub name, summary or description. Leave the tag blank for build-only
runs. Do not use this retry for a version already successfully uploaded.
