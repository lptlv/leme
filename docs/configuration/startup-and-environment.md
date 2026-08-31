# Startup and environment

The `env` block sets variables for programs launched by Leme. It supplies values to child processes, but does not change Leme's own environment or the parent login session:

```scfg
env {
    MOZ_ENABLE_WAYLAND 1
}
```

By contrast, `$(env.NAME)` reads Leme's own process environment only while loading or reloading the configuration. Neither feature invokes a shell. Environment reads located in inactive static conditional branches are not evaluated.

## Setting configuration inputs

`leme.env` beside the configuration file supplies values for `$(env.NAME)`
reads. It is the portable way to give one configuration a per-machine value
without depending on how the session is started, which differs between
distributions and login managers.

It does not export anything. Variables for the programs Leme launches belong in
the `env` block, which is unrelated: `leme.env` is what the configuration
reads, `env` is what the session exports.

The pointer theme and size belong in the [cursor](cursor.md) block instead. XWayland does not inherit what `env` sets.

The `exec` block starts each argument vector once after startup:

```scfg
exec {
    foot --server
    waybar
}
```

A configuration reload does not rerun `exec` entries.

`spawn` and `exec` call programs directly without a shell. Leme expands its
own `$name` configuration variables, but it does not expand `~`, shell
environment variables, pipes, redirections, command substitution, or globs.
Use `$$VARIABLE` to pass a literal environment reference to an explicitly
invoked shell. Use an absolute path when the executable is not on `PATH`. The
[syntax and variables](syntax-and-variables.md) page has a complete shell
example.

After applying the `env` block, Leme sets `WAYLAND_DISPLAY` and `DISPLAY` to
the compositor's active values. Those two names cannot be overridden through
`env`. Leme also exports `LEME_SOCKET` when the control socket is available.
The [timao reference](../reference/timao.md) explains how the client uses it.
