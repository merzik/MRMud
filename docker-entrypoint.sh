#!/usr/bin/env bash

set -euo pipefail

GAME_DIR=/game
SEED_DIR=/usr/local/share/mrmud-seed
DEFAULT_PORT=${MRMUD_PORT:-${MR_PORT:-4321}}

directory_is_empty() {
  local dir=$1

  shopt -s nullglob dotglob
  local entries=("$dir"/*)
  shopt -u nullglob dotglob

  [[ ${#entries[@]} -eq 0 ]]
}

seed_areas_if_empty() {
  mkdir -p "$GAME_DIR/areas"

  if directory_is_empty "$GAME_DIR/areas"; then
    cp -a "$SEED_DIR/areas/." "$GAME_DIR/areas/"
  fi
}

prepare_player_directory() {
  local letter

  mkdir -p "$GAME_DIR/player"

  for letter in {a..z}; do
    mkdir -p "$GAME_DIR/player/$letter/bak"
  done

  if [[ ! -e "$GAME_DIR/player/c/Chaos" ]]; then
    cp "$SEED_DIR/player/c/Chaos" "$GAME_DIR/player/c/Chaos"
  fi
}

prepare_runtime_directories() {
  mkdir -p "$GAME_DIR/log"
  seed_areas_if_empty
  prepare_player_directory
}

prepare_runtime_directories

if [[ $# -eq 0 ]]; then
  set -- "$GAME_DIR/src/startup.bash" "$DEFAULT_PORT"
elif [[ $1 == "startup" ]]; then
  shift
  if [[ $# -eq 0 ]]; then
    set -- "$GAME_DIR/src/startup.bash" "$DEFAULT_PORT"
  else
    set -- "$GAME_DIR/src/startup.bash" "$@"
  fi
elif [[ $1 =~ ^[0-9]+$ ]]; then
  set -- "$GAME_DIR/src/startup.bash" "$1"
fi

exec "$@"
