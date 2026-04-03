#!/usr/bin/env bash

set +e

port=4321
if [[ -n "${1:-}" ]]; then
  port="$1"
fi

cd "$(dirname "$0")/../areas" || exit 1

ulimit -a >> /dev/null 2>&1

while true; do
  [[ -e shutdown.txt ]] && rm -f shutdown.txt

  echo "Finding log file..."
  index=1000
  while true; do
    logfile="../log/${index}.log"
    [[ ! -e "$logfile" ]] && break
    ((index++))
  done

  echo "Found new log file $logfile!"
  date > "$logfile"
  date > ../areas/boot.txt
  ulimit -a >> "$logfile" 2>&1

  echo "Checking for binary file..."

  cp -f ../bin/next_md ../bin/current_md
  if [[ ! -e ../bin/current_md ]]; then
    echo "There is no binary file."
    echo "Be sure to compile, and copy ../bin/md to ../bin/next_md."
    exit 0
  else
    echo "Binary file OK!"
  fi

  ../bin/current_md "$port" >> "$logfile" 2>&1

  corefile=""
  for candidate in core*; do
    if [[ -e "$candidate" ]]; then
      corefile="$candidate"
      break
    fi
  done

  if [[ -n "$corefile" ]]; then
    if [[ -x /usr/bin/gdb ]]; then
      printf 'where\n' | /usr/bin/gdb ../bin/md "$corefile" >> "$logfile" 2>&1
      printf '\n' >> "$logfile"
    else
      echo "gdb not found, skipping core analysis." >> "$logfile"
    fi

    rm -f core*
  fi

  if [[ -n "${MR_CRASH_MAIL_TO:-}" && -x /usr/bin/mail ]]; then
    tail -50 "$logfile" | /usr/bin/mail -s "MR Crash Info" "$MR_CRASH_MAIL_TO"
  fi

  sleep 30
done
