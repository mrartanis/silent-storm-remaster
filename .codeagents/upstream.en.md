# Import fixes from upstream

[Русский](upstream.md) · [Task index](README.en.md)

1. Check the current branch and remotes. Compare the upstream range against `develop`, the [import register](../diagnostics/upstream-import-status.tsv), and the `Upstream-Commit:` trailers in Git history. Compare the implementation and tests when a change looks equivalent. Adapted imports have different commit hashes.
2. Split mixed diffs into independent fixes. Preserve the original balance, intentional fork behavior, and compatibility. Compare uncertain gameplay changes with the original game. Record existing equivalents, imported fixes, and deliberate exclusions separately.
3. For each fix, get the intended RED result on the production implementation and GREEN after the change. A build failure does not demonstrate a gameplay bug. Check the affected platforms, serialization, campaign, or multiplayer as appropriate.
4. When committing, keep the test and fix together in one focused commit with an `Upstream-Commit: <public commit URL>` trailer. Update the import register. If the user requested a push, publish the agreed changes and verify the remote branch.

[Balance review](../diagnostics/UPSTREAM-BALANCE-CHANGES.md) · [CI test registration](../diagnostics/UPSTREAM-TEST-REGISTRATION.md)
