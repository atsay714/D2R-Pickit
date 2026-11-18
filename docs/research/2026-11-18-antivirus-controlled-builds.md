# Antivirus comparison for D2R Pickit 0.12.1

The 0.12.0 detection follows the compiled export-formatting path in controlled
Windows builds. Removing its parser, file I/O or file dialog independently
retains the same Ulise detection. Disabling export formatting removes it.
The complete 0.12.1 candidate preserves the features, simplifies the fixed
export schema and scans 0/71 on 2026-11-18, including Bitdefender.

These experiments identify a component associated with the result. They do
not disclose Bitdefender's proprietary matched bytes or establish that toml++
is malicious. Vendor review of the original file remains case 01198433.

## Reproducible build controls

[First comparison run](https://github.com/atsay714/D2R-Pickit/actions/runs/44248366911)
built four variants sequentially on one Windows runner, using the same clean
source/build location and MSVC 19.51.36260.0. Both unmodified controls reproduced
the released DLLs and native ZIPs byte for byte:

| Control | DLL SHA-256 | Native ZIP SHA-256 |
| --- | --- | --- |
| 0.11.0 | `0d88856b068c8840c650d7d204befa8e4d557ab38e7ad0d94bde596ce8467609` | `58baba5a4ad7ad34216493a55a5d0a6006b98458bb3b1e0ef6620e4e202abded` |
| 0.12.0 | `620c0c18a98efc2440d929f998fe5fc0f3d5f6e9d8ea6e03eddb43c0fef7fdc0` | `56acbf6c8e65d72fbdcfef3371e3ccc280100223b99aaf25aceff4aa3f33cf4e` |

The radius-only variant changes gameplay/config range handling on the 0.11.0
source and retains its old settings frontend. The import/export variant keeps
the 0.12.0 import/export and window changes but restores the distance limit to
20. Test expectations follow each variant's intended distance limit. The old
and radius-only variants passed 27 CTests each; the two newer variants passed
29 each. All four passed seven packaging tests.

## Component elimination

[Second comparison run](https://github.com/atsay714/D2R-Pickit/actions/runs/44249315591)
disabled one component at a time in 0.12.0. These deliberately incomplete
samples were compiled and package-checked, with behavioral tests disabled.
Their full control still reproduced the original DLL hash. None is a release
candidate. Source patches and hashes are retained in the Actions artifacts;
the generator is on branch `codex/antivirus-controlled-builds` at `68a100d`.

| DLL variant | VirusTotal detections | Bitdefender |
| --- | --- | --- |
| [0.11.0 released DLL](https://www.virustotal.com/gui/file/0d88856b068c8840c650d7d204befa8e4d557ab38e7ad0d94bde596ce8467609) | 0/70, Nov 17 | Undetected |
| [0.12.0 released DLL](https://www.virustotal.com/gui/file/620c0c18a98efc2440d929f998fe5fc0f3d5f6e9d8ea6e03eddb43c0fef7fdc0) | 8/71, Nov 18 | Gen:Variant.Ulise.601194 |
| [Radius core only](https://www.virustotal.com/gui/file/1bd1cb737446cee6a64e0d8808c94fefbf37f3f5374298404794a0959661a0b9) | 0/70, Nov 18 | Undetected |
| [Import/export with distance bound 20](https://www.virustotal.com/gui/file/6a4986ba624b4141f9c4e30a806a338076ae09c5bb0caa845e38940fdd158fac) | 8/71, Nov 18 | Gen:Variant.Ulise.601194 |
| [Without rule parser](https://www.virustotal.com/gui/file/ef8888eadbdc89e4876eaa2e934a3eec335bc9066a43b068e3b74aa7f9b2011a) | 8/71, Nov 18 | Gen:Variant.Ulise.601194 |
| [Without rule file I/O](https://www.virustotal.com/gui/file/232156d589d862b8582fb5d1ad1c467853a3af04bdecd534d3365ad4ce70c6a3) | 8/71, Nov 18 | Gen:Variant.Ulise.601194 |
| [Without file dialog](https://www.virustotal.com/gui/file/9691dd8d2315f186ae54660fbc24f06b9fc2338a4d09e4cd29821108e81b0ae4) | 8/71, Nov 18 | Gen:Variant.Ulise.601194 |
| [Without export formatter](https://www.virustotal.com/gui/file/53c002808742902a0b2baeb55e256acdb767dd500cb423820e0e0c0453fec251) | 0/70, Nov 18 | Undetected |

Counts describe the observed completed reports, rather than a permanent
verdict. The original ZIP's Bitdefender scan timed out; direct DLL comparisons
avoid treating that archive result as a clean DLL scan.

## Production correction

The export has fixed keys, booleans and bounded item-code arrays. Only its
informational mod-name string needs general string escaping. Version 0.12.1
writes that schema directly, escapes quotes/backslashes and ASCII controls as
TOML basic-string content, and retains UTF-8 metadata. General-purpose toml++
formatters are disabled; its full parser remains available for imports from
older or future config schemas. File dialogs, backups, atomic publication,
active-config protection and pickup distance through 9999 are unchanged.

A new fixture also reproduced malformed exported metadata in the old code
when a mod name included newlines and both triple-quote delimiters. It failed
against the old exporter and passes against the new implementation. Other
fixtures verify empty names, quotes/backslashes, all ASCII controls and Unicode
through the real TOML parser, then import and compare the rule selections.

The [0.12.1 preflight](https://github.com/atsay714/D2R-Pickit/actions/runs/44250788082)
passed 29 Windows CTests and seven packaging tests. Its full
[DLL scan](https://www.virustotal.com/gui/file/7c0910be139f6713f7e683e4f3d7f1dfcc24ee1712999147511348436679e099)
returned 0/71. The DLL is 602112 bytes, compared with 747008 bytes in 0.12.0,
a reduction of 144896 bytes, about 19.4 percent.
The corresponding native ZIP is
`f128349080baf587861e38006d5a52e054a9ea29af48792fd6a5930020e5c6f0`;
its [completed scan](https://www.virustotal.com/gui/file/f128349080baf587861e38006d5a52e054a9ea29af48792fd6a5930020e5c6f0)
returned 0/67 on the same date.

## Related reports

The [MSYS2/QEMU Ulise report](https://github.com/msys2/MINGW-packages/issues/29838)
records a maintainer submitting a false-positive report and subsequently
reporting that Bitdefender expected a correction. Other native projects,
including [llama.cpp](https://github.com/ggml-org/llama.cpp/issues/26739), have
reported the same generic family with different numeric labels. These reports
do not identify our signature's trigger. Searches found no specific upstream
toml++ antivirus report matching this case.

[toml++ PR 323](https://github.com/marzer/tomlplusplus/pull/323) separately
describes the multiline-string serialization defect exposed by the new
fixture. The inspected PR is closed without a merge; it is evidence of the
reported formatting issue, not an antivirus explanation or an upstream fix
included in the pinned 3.4.0 header.
