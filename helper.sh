#!/usr/bin/env bash
set -eu
program="$0"

usage() {
  cat <<EOF
Run benchmark-io while mesuring system performance with sysstat.

Usage:
  $program <bin> <arg>...
  $program -h | --help

Arguments:
  bin  path to benchmark-io binary
  arg  arguments passed to benchmark-io

EOF
}

main() {
  local workdir="${@:$#}"
  local sar_out="$workdir/benchmark-io_sar_$(date +%Y-%m-%d-%H-%M-%S).out"

  echo "$program: Output sar report in binary form to $sar_out"
  echo "$program: Run benchmark-io"
  echo ""

  numactl -C 0 -- $@ &
  local bench_pid=$!

  sar -u -P 0 -o "$sar_out" 1 > /dev/null &
  local sar_pid=$!

  wait $bench_pid
  kill -INT $sar_pid
}


if [[ $# -eq 0 ]]; then
  usage
  exit 1
elif [[ "$1" = "-h" || "$1" = "--help" ]]; then
  usage
else
  main $@
fi
