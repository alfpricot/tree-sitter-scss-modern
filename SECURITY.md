# Security policy

## Reporting a vulnerability

Do not open a public issue for a vulnerability. Private vulnerability reporting is not enabled on this
repository, so contact the maintainer, [@alfpricot](https://github.com/alfpricot), directly through their
GitHub profile, and include what you found, how to reproduce it, and the impact you expect.

A parser crash or hang on some input that has no wider impact can go in a normal GitHub issue: include the
input, or a minimized version, and which binding you used (Rust or Node.js).

## Scope

This policy covers the grammar, the external scanner, and the Rust and Node.js bindings in this
repository. A defect in the tree-sitter runtime or CLI belongs to the
[tree-sitter project](https://github.com/tree-sitter/tree-sitter); a defect that also exists in the
upstream grammar belongs to
[`tree-sitter-grammars/tree-sitter-scss`](https://github.com/tree-sitter-grammars/tree-sitter-scss).

## Supported versions

Only the latest published release receives fixes.

## Practices

- Do not commit credentials, tokens, or keys; the `Verify` workflow scans the working tree and history
  for secrets.
- Keep the committed parser (`src/parser.c`) generated from `grammar.js` by the pinned
  `tree-sitter-cli` version; the `Verify` workflow fails on any difference.
- Dependabot opens weekly update pull requests against `dev`; review and merge them rather than letting
  dependencies age.
