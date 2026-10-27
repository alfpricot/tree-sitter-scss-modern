# Contributing to tree-sitter-scss-modern

A tree-sitter grammar for current SCSS syntax, with Rust and Node.js bindings.

## Before you start

1. Branch from **`dev`**. `main` is the production branch; a release pull request promotes `dev` into it.
2. Read [ARCHITECTURE.md](ARCHITECTURE.md) for what each directory holds.

## Local setup

```sh
npm install --ignore-scripts
npm rebuild tree-sitter-cli
npx --yes --package=tree-sitter-cli@0.27.0 -- tree-sitter generate
cargo build
```

`--ignore-scripts` skips the CLI's binary download, so `npm rebuild tree-sitter-cli` fetches it. The
generated parser (`src/parser.c`, `src/grammar.json`, and `src/node-types.json`) is committed. Regenerate
it and commit the diff after every `grammar.js` change. On Windows without MSVC, set `CC` and `CXX` to an
installed GCC-compatible toolchain (for example `CC=gcc CXX=g++`).

## Validation before a pull request

The `Verify` workflow runs these checks:

```sh
npx --yes --package=tree-sitter-cli@0.27.0 -- tree-sitter generate
node test/generate-node-kinds.mjs
npm run lint
npm run format:check
node --test tools/*.test.mjs
npx --yes --package=tree-sitter-cli@0.27.0 -- tree-sitter test
npx --yes --package=tree-sitter-cli@0.27.0 -- tree-sitter parse examples/*.scss
cargo fmt --check
cargo clippy --all-targets -- -D warnings
cargo test
npm test
```

- The workflow fails when `tree-sitter generate` or `node test/generate-node-kinds.mjs` leaves a diff, so
  commit `src/parser.c`, `src/grammar.json`, `src/node-types.json`, and `test/node-kinds.txt` with the
  grammar change.
- `test/corpus/` holds the tree assertions, including the SCSS constructs from the original probe list.
  Add a corpus test for every grammar change.
- Every file in `examples/` must parse with no `ERROR`, `MISSING`, or zero-width node.
- `npm run format` applies formatting; the generated parser under `src/` is excluded.
- `node test/parse-rate.mjs <directory>` lists the `.scss` files under a directory whose tree contains an
  error, which is useful for checking a change against a larger body of stylesheets.

CI also validates the pull request title, scans for secrets, and lints the workflow and YAML files.

## Pull requests

- Open pull requests against **`dev`**.
- Use a Conventional Commit title (`feat:`, `fix:`, `docs:`, `chore:`, `test:`, ...).
- Keep each pull request to one change, and link the issue it resolves in the body.
- Release-plz computes versions and `CHANGELOG.md` from commit messages, so do not edit either by hand.
