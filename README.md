# tree-sitter-scss-modern

[![Verify](https://github.com/alfpricot/tree-sitter-scss-modern/actions/workflows/verify.yml/badge.svg?branch=dev)](https://github.com/alfpricot/tree-sitter-scss-modern/actions/workflows/verify.yml)
[![crates.io](https://img.shields.io/crates/v/tree-sitter-scss-modern)](https://crates.io/crates/tree-sitter-scss-modern)

A [tree-sitter](https://tree-sitter.github.io/tree-sitter/) grammar for current SCSS syntax.

This project is a maintained fork of
[`tree-sitter-grammars/tree-sitter-scss`](https://github.com/tree-sitter-grammars/tree-sitter-scss), licensed
under MIT. It adds the Sass module system (`@use` and `@forward` options), maps, variable flags, boolean
operators, and other current syntax the original does not parse, keeping the original's node kinds.

## Installation

For Rust, add the crate next to `tree-sitter`:

```sh
cargo add tree-sitter tree-sitter-scss-modern
```

For Node.js, install the package next to `tree-sitter`:

```sh
npm install tree-sitter-scss-modern
```

A GitHub Packages copy is also published as `@alfpricot/tree-sitter-scss-modern`.

### Requirements

- Rust 1.85 or later (`rust-version` in `Cargo.toml`) for the Rust binding.
- Releases ship prebuilt native addons for Linux and Windows, so an npm install there needs no C
  toolchain. On any other platform, `node-gyp` builds the addon from source and needs a C compiler.

## Usage

Load the language into a parser from Rust:

```rust
let mut parser = tree_sitter::Parser::new();
let language = tree_sitter_scss_modern::LANGUAGE;
parser
    .set_language(&language.into())
    .expect("Error loading current SCSS syntax parser");
```

or from Node.js:

```js
import Parser from 'tree-sitter';
import Language from 'tree-sitter-scss-modern';

const parser = new Parser();
parser.setLanguage(Language);
```

The package also ships `queries/highlights.scm` for syntax highlighting.

## How to contribute

Branch from `dev` and open the pull request against `dev`. Regenerate the committed parser after every
`grammar.js` change and add a corpus test for it. [CONTRIBUTING.md](CONTRIBUTING.md) lists the build,
test, and lint commands, and [ARCHITECTURE.md](ARCHITECTURE.md) describes how the pieces fit together.

## License

MIT. See [LICENSE](LICENSE) and [NOTICE](NOTICE).
