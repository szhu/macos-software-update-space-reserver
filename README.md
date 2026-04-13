# macOS Software Update Space Reserver

Reserve disk space for macOS updates and whenever else you quickly need it.

## Why

Ever hit "Not enough free space" when you're trying to install a software update, and not even sure how your disk filled up in the first place?

This tool prevents that problem from happening in the first place by creating a large empty file, to make sure that space is there when you need it!

In addition, this repo also includes a helper tool for installing macOS software updates, that will:

1. shrink the reserved-space file just the right amount
2. install the updates
3. re-reserve the space after the computer restarts after the update.

## `reserve-space`

Manages an empty file at `~/Library/Reserved Space/reserved` whose only job is to take up space.

```sh
bin/reserve-space             # show current reservation and disk status
bin/reserve-space 45g         # reserve exactly 45 GB
bin/reserve-space --free 20g  # reserve so that 20 GB remains free
bin/reserve-space --clear     # remove the reservation
```

## `reserve-space-softwareupdate`

A wrapper around `softwareupdate` that automatically frees the reservation before installing, then restores it after reboot.

```sh
bin/reserve-space-softwareupdate --list
bin/reserve-space-softwareupdate --all --install
sudo bin/reserve-space-softwareupdate --all --install --restart  # --restart requires root
```

<details>
<summary>Why is this script so complicated?</summary>

These issues were discovered through trial and error while building this tool. See the git history and inline code comments for the full story.

- **Exit code**: `softwareupdate --install` exits 0 even when it prints "Not enough free disk space: a total of X GB is required." The only way to detect this is to capture and parse stdout/stderr.

- **Output buffering**: When stdout/stderr is a pipe (even through `tee`), `softwareupdate` switches from line-buffered to block-buffered mode. All output is withheld until the process exits. A PTY makes it think it's writing to a real terminal, keeping output line-buffered.

- **Password prompt**: `softwareupdate --install` prompts for a password by opening `/dev/tty` directly, bypassing stdout/stderr. When run as a subprocess via `Process()`, this prompt is invisible and unresponsive, causing the process to hang silently. Using `exec()` to replace the wrapper process with `softwareupdate` gives it direct terminal access.

- **Root requirement**: `--restart` requires root. Running the whole wrapper with `sudo` is supported; `SUDO_USER` is used to find the real user's home directory.

</details>
