# Regex

This document contains all features in Soup's flavour of Regular Expressions.

If there's a feature that's not mentioned here, feel free to open an issue on our Github page.

## Flags

- 'm' — '^' and '$' also match start and end of lines, respectively
- 's' — '.' also matches '\n'
- 'i' — Case insensitive match
- 'x' — Ignore bare space characters in pattern. '#' signifies begin of line comment.
- 'u' — Treat pattern and strings-to-match as UTF-8 instead of binary data
- 'U' — Quantifiers become lazy by default and are instead made greedy by a trailing '?'
- 'D' — '$' only matches end of pattern, not '\n' — ignored if 'm' flag is set
- 'n' — Only capture named groups (non-standard flag from .NET/C#)

## Encoding

soup::Regex only operates on bytes/chars, which means that `💯` matches `/^....$/` (assuming UTF-8).

Note that some environments, like JavaScript, operate on words (UTF-16), so `💯` matches `/^..$/` instead.

With the 'u' flag, we pretend to operate on Unicode codepoints, and Soup specifically will assume that the input is UTF-8-encoded, which means that `💯` matches `/^.$/u` in all environments.

## Groups

- A regex is a capturing group with id 0.
- You can define additional capturing groups with `(` ... `)`; their ids will be sequential.
- You can name capturing groups by doing `(?'name'` ... `)` or `(?<name>` ... `)`.
- Anonymous groups may be non-capturing: `(?:` ... `)`.
- Anonymous groups can also set modifiers that only apply within them, e.g. to enable 'i' flag and disable 's' flag: `(?i-s:` ... `)`
	- There is an alternative syntax of this that changes the flags globally: `/(?i)abc/` is identical to `/abc/i`.

## Constraints

- Start constraint (`^`): Assert position at start of sequence (with 'm' flag: or start of a line)
- End constraint (`$`): Assert position at end of sequence (with 'm' flag: or end of a line) (without 'D' flag: or the '\n' before the end of the sequence)
- "Any" constraint (`.`): Matches any byte (with 'u' flag: any codepoint) except for '\n' (unless 's' flag is given)
- Range constraint (`[` ... `]`): Matches any byte in the range, which may be defined as multiple bytes (`[abc]`), a range of bytes (`[a-c]`), a class (`[[:alnum:]]`), or a mixture of all of these (`[[:alpha:]0-9 ]`)
	- Classes: `[:alnum:]`, `[:alpha:]`, `[:ascii:]`, `[:blank:]`, `[:cntrl:]`, `[:digit:]`, `[:graph:]`, `[:lower:]`, `[:upper:]`, `[:word:]`, `[:xdigit:]`
- Byte constraint: Matches the byte precisely. This is any byte in the pattern that doesn't have a special meaning as described in this document.
- Recall constraint (`\index`, `\k'name'`, or `\k<name>`): Matches for the contents of a previous capturing group, e.g. `(.)\1` matches any instance of a repeated byte.

### Escape Sequences

- `\b` — Assert position is at word boundary
- `\B` — Assert position is not at word boundary
- `\w` — Match a word character
- `\W` — Match a non-word character
- `\A` — Assert position at start of sequence (same as `^` without 'm' flag)
- `\Z` — Assert position at end of sequence, or the '\n' before the end of the sequence (same as `$` without 'm' flag)
- `\z` — Assert position at end of sequence (same as `$` with the 'D' flag)
- `\d` — Match a digit (same as `[0-9]`)
- `\s` — Match a whitespace character

Any other character preceeded by a backslash (`\`) is treated as a byte constraint.

### Meta

- `?`: Preceeding constraint is optional (match between 0-1 times).
- `*`: Preceeding constraint can match as many times as it wants.
	- `*?`: As few times as possible, expanding as needed (lazy).
- `+`: Preceeding constraint must match at least 1 time and can match as many times as it wants.
	- `+?`: As few times as possible, expanding as needed (lazy).
- `{x}`: Preceeding constraint must match exactly x times.
- `{x,}`: Preceeding constraint must match at least x times and can match as many times as it wants.
	- `{x,}?`: As few times as possible, expanding as needed (lazy).
- `{x,y}`: Preceeding constraint must match at least x times and at most y times.
	- `{x,y}?`: As few times as possible, expanding as needed (lazy).

### Lookahead & Lookbehind

These are constraints that don't change the position:

- `a(?=b)` — Checks if the current position matches `a` and the next position matches `b`.
- `a(?!b)` — Checks if the current position matches `a` and the next position does not match `b`.
- `(?<=a)b` — Checks if the current position matches `b` and the previous position matches `a`. Lookbehind group must be fixed-width.
- `(?<!a)b` — Checks if the current position matches `b` and the previous position does not match `a`. Lookbehind group must be fixed-width.

## Alternatives

`|` indicates an alternative for the current group, e.g. `apple|banana` matches both `apple` and `banana`.
