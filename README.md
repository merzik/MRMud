# Mortal Realms

Source code and game data for the Mortal Realms MUD.

Official website: [mortalrealms.com](https://mortalrealms.com)

## Overview

This repository contains:

- The C game server in `src/`
- Area files and runtime data in `areas/`
- Compiled binaries in `bin/`
- Player data in `player/`
- Clan data in `clans/`
- Runtime logs in `log/`

The server is a classic Diku/Merc-derived MUD codebase and expects to run with the repository's directory structure intact.

## Prerequisites

You need:

- `make`
- `gcc` or a compatible C compiler
- Standard Unix development tools

On Linux, the Makefile links with `-lcrypt`, so you may need the system crypt library development package installed.

The startup scripts also optionally use:

- `gdb` to inspect crash dumps
- `/usr/bin/mail` to send crash notifications

## Building

Build from the `src/` directory:

```bash
cd src
make
```

This produces the main server binary at `bin/md`.

The Makefile also includes an AddressSanitizer target:

```bash
cd src
make asan
```

## Running

### Docker

Build the image:

```bash
docker build -t mrmud .
```

Build and publish a multi-platform image with x64 and ARM64 support:

```bash
docker buildx build \
  --platform linux/amd64,linux/arm64 \
  -t your-registry/mrmud:latest \
  --push \
  .
```

GitHub Actions also publishes multi-platform images to GitHub Container Registry:

- Pushes to `main` publish `ghcr.io/merzik/mrmud:latest`
- Pushes to `dev` publish `ghcr.io/merzik/mrmud:dev`

Run the game on the default port:

```bash
docker run --rm -it \
  -p 4321:4321 \
  -v "$PWD/log:/game/log" \
  -v "$PWD/player:/game/player" \
  -v "$PWD/areas:/game/areas" \
  mrmud
```

Example `compose.yaml`:

```yaml
services:
  mrmud:
    image: ghcr.io/merzik/mrmud:latest
    ports:
      - "4321:4321"
    environment:
      MRMUD_PORT: "4321"
    volumes:
      - ./log:/game/log
      - ./player:/game/player
      - ./areas:/game/areas
    restart: unless-stopped
```

The default port is `4321`. To use another port, set `MRMUD_PORT` and publish
the same host/container port:

```bash
docker run --rm -it \
  -e MRMUD_PORT=4444 \
  -p 4444:4444 \
  mrmud
```

The image stores the repository at `/game`. Runtime data can be persisted by
mounting `/game/log`, `/game/player`, and `/game/areas`.

On startup, the container creates missing `player/a` through `player/z`
directories with `bak` children and seeds `player/c/Chaos` when it is missing.
If `/game/areas` is mounted and empty, the container seeds it from the image.
Files named `.gitkeep` are excluded from the image.

You can also pass the port as the container command:

```bash
docker run --rm -it -p 4444:4444 mrmud 4444
```

### Recommended: use a startup script

The startup scripts handle:

- Changing into the `areas/` directory before launch
- Copying `bin/next_md` to `bin/current_md`
- Writing boot information to `areas/boot.txt`
- Logging output to `log/*.log`
- Restarting the server if it exits

After building, copy the binary into the deploy slot expected by the launcher:

```bash
cp bin/md bin/next_md
```

Then start the server with either script:

```bash
./src/startup.zsh
```

or:

```bash
./src/startup.bash
```

Default port is `4321`. To use a different port:

```bash
./src/startup.zsh 4444
```

If you want crash emails from the Bash/Zsh launchers, set `MR_CRASH_MAIL_TO` before starting:

```bash
export MR_CRASH_MAIL_TO=you@example.com
./src/startup.zsh
```

### Run the binary directly

The server expects to be started from the `areas/` directory so it can find files like `area.lst`, `siteban.lst`, and other runtime data.

```bash
cd areas
../bin/md
```

To bind to a non-default port:

```bash
cd areas
../bin/md 4444
```

## Runtime Notes

- Default game port is `4321`
- Log files are created in `log/`
- The startup wrappers use `bin/current_md` as the active runtime binary
- Game content and persistent data are loaded from the repository directories directly

Because the launcher auto-restarts the server, direct binary execution is usually better for local debugging.

## Repository Layout

```text
MRMud/
├── areas/   # area files and runtime data
├── bin/     # compiled server binaries
├── clans/   # clan data
├── log/     # runtime logs
├── player/  # player save files
└── src/     # C source, Makefile, and startup scripts
```
