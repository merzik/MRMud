#!/usr/bin/env bash

set -euo pipefail

GAME_DIR=/game
SEED_DIR=/usr/local/share/mrmud-seed
DEFAULT_PORT=${MRMUD_PORT:-${MR_PORT:-4321}}
RUN_UID=${MRMUD_UID:-}
RUN_GID=${MRMUD_GID:-}

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

validate_id() {
  local name=$1
  local value=$2

  if [[ ! $value =~ ^[0-9]+$ ]]; then
    echo "$name must be a numeric id, got: $value" >&2
    exit 1
  fi
}

runtime_user_requested() {
  [[ -n $RUN_UID || -n $RUN_GID ]]
}

prepare_runtime_user() {
  if [[ -z $RUN_UID ]]; then
    echo "MRMUD_UID must be set when MRMUD_GID is set" >&2
    exit 1
  fi

  if [[ -z $RUN_GID ]]; then
    RUN_GID=$RUN_UID
  fi

  validate_id MRMUD_UID "$RUN_UID"
  validate_id MRMUD_GID "$RUN_GID"

  chown -R "$RUN_UID:$RUN_GID" \
    "$GAME_DIR/areas" \
    "$GAME_DIR/bin" \
    "$GAME_DIR/log" \
    "$GAME_DIR/player"
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

if runtime_user_requested; then
  prepare_runtime_user
  exec gosu "$RUN_UID:$RUN_GID" "$@"
fi

exec "$@"
