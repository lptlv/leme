# Syntax and variables

Leme's configuration uses scfg blocks. A block name may be followed by a quoted name and then a brace-delimited list of directives:

```scfg
vars {
    mod SUPER
    term foot
}
```

Unknown blocks, directives, duplicate keys, invalid values, and malformed bindings produce configuration diagnostics. See [configuration errors](config-errors.md) for which problems are recoverable.

A syntax error reports the line, the column, and the offending line with a
caret. Leme reports every syntax error in one pass rather than stopping at the
first, so a file with several mistakes needs one round of fixes rather than
one reload per mistake. A file with any syntax error is refused outright and
the previous configuration stays active.

## One-line blocks

A block may open and close on the same line. A directive ends at a newline, so
an unquoted `;` is used to fit more than one directive on a line:

```scfg
exec { waybar }
vars { mod SUPER; term foot }
scratchpad "term" { width 0.5; height 0.6 }
```

Without the separator, `{ width 0.5 height 0.6 }` would be a single `width`
directive with three parameters. A quoted `;` stays literal, so `foo "a;b"`
passes `a;b` through unchanged.

## Comments

A `#` begins a comment when it is followed by a space, a tab, or the end of
the line, wherever it appears:

```scfg
exec {
    waybar --bar main # start the bar
}
```

The condition matters because colours are written `#RRGGBB`. A `#` followed
directly by another character is an ordinary part of a word, so
`border_active #296bb8` keeps its value whether or not it is quoted. Write
comments as `# text`, with a space, as every example here does.

## Variables

A `vars` block names values for reuse anywhere in the file:

```scfg
vars {
    mod SUPER
    term foot
    browser firefox
}

binds "common" {
    $mod+Return spawn $term
    $mod+b spawn $browser
}
```

`$name` expands in directive names and parameters. That is why `$mod+q` works: the key specification is the binding's directive name.

A name consumes the longest run of letters, digits, and underscores. Use `$(name)` when a suffix begins with one of those characters:

```scfg
vars {
    browser firefox
    nightly $(browser)nightly
}
```

`$(name)` is variable syntax, not command substitution. `$$` produces a literal `$`. An unknown variable is an error, not an empty string.

The `vars` block may appear once anywhere in the file. Names declared earlier in the block are visible to later names. Leme collects the block before expansion, so a variable may be used above its declaration in another block.

## Environment reads

`$(env.NAME)` reads an environment variable from Leme's parent process during configuration load and reload:

```scfg
vars {
    terminal $(env.TERMINAL)
}
```

If the named environment variable is unset or empty, Leme records a recoverable diagnostic and substitutes the literal string `none`. The value is inserted literally into the field without rescanning syntax. Environment reads located inside inactive conditional branches are never evaluated.

If a variable is not set in Leme's environment, `$(env.NAME)` falls back to an
optional `leme.env` file beside the configuration file. The file is plain
`NAME=value` lines:

```
# ~/.config/leme/leme.env
LEME_PROFILE=desktop
PRIMARY_OUTPUT=DP-1
```

Blank lines are ignored, and a line whose first non-blank character is `#` is a
comment. Blanks around the name and the value are trimmed. The value is taken
literally: quotes are kept, `$` is not expanded, and `export NAME=value` is
rejected. An empty value counts as unset.

The process environment always wins over the file, so a value can be overridden
for one run without editing anything:

```sh
LEME_PROFILE=laptop leme --config-check
```

The file is read from the directory of the configuration file being loaded, and
it is re-read on `reload_config`, so changing it does not require a new session.
An absent file is not an error.

Environment reads always require the explicit `$(env.NAME)` syntax. Regular `$name` and `$(name)` forms look up configuration variables and loop locals. `$env.NAME` does not read the process environment; it is treated as a configuration variable `$env` followed by the literal text `.NAME`. Without a configuration variable named `env`, `$env.NAME` fails as an unknown variable error. Use `$$` to produce a literal `$` character.

## Ranges and loops

A `for` loop iterates over an inclusive ascending integer range or a named list:

```scfg
for i in 1..12 {
    CTRL+ALT+F$i switch_vt $i
}
```

Range bounds must be nonnegative integers with `low <= high`. The loop variable exists only within the loop body and shadows any same-named scalar or outer loop variable.

Loops may be nested. Template generation enforces four safety bounds:

- at most 16 simultaneously active generation levels across nested `if` and `for` blocks;
- at most 1,024 iterations per single loop;
- at most 65,536 total loop iterations across the entire configuration expansion;
- at most 8,192 total directives emitted across the expanded configuration.

Descending ranges, custom step sizes, and arithmetic expressions are not supported. See [Reading expansion notes](config-errors.md#reading-expansion-notes) for how diagnostics trace generated directives back to their loops.

## Named lists

A single top-level `lists` block declares named collections of string items:

```scfg
lists { pads "term" "music" "notes" }
for pad in pads {
    scratchpad "$pad" { identity "foot-$pad" }
}
```

Items are expanded in declaration order. Lists and scalar variables occupy separate namespaces, allowing a list and scalar to share a name. List items undergo scalar and environment interpolation when the `lists` block is collected. Inline list literals and record objects are not supported.

## Static conditions

Static conditionals compare strings at load time using `==` or `!=`:

```scfg
vars { profile $(env.LEME_PROFILE) }
if $profile == "laptop" {
    output "eDP-1" { scale 1.25 }
} else {
    output "DP-1" { scale 1.0 }
}
```

An `if` block may be followed by an optional adjacent `else` block. Conditionals can be nested inside either branch. Only the selected branch is expanded and checked for semantic validity; unselected branches do not evaluate variables or report semantic diagnostics. Structural syntax errors, such as unclosed quotes or missing braces anywhere in the file, are rejected during initial parsing before expansion runs.

## Language boundary

Template generation occurs entirely at configuration load time. Leme does not execute shell scripts, run-time conditional hooks, functions, macros, or command substitutions during template expansion.

## Shell syntax is not implicit

`spawn` and `exec` receive an argument vector directly after Leme expands its
own `$name` configuration variables. Leme does not perform shell expansion for
`~`, environment variables, pipes, redirections, command substitution, or
globs. Write `$$HOME` when the child should receive the literal text `$HOME`.

If a program needs shell syntax, invoke a shell explicitly:

```scfg
binds "common" {
    SUPER+x spawn sh -c "printf '%s\\n' \"$$HOME\" >\"$$XDG_RUNTIME_DIR/example\""
}
```

The [startup and environment](startup-and-environment.md) page covers `env`, `exec`, and child process behavior.
