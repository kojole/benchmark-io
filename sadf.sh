#!/usr/bin/env bash
set -eu
program="$0"

usage() {
  cat <<EOF
Convert sar output of helper.sh to tsv.

Usage:
  $program SAR_OUT
  $program -h | --help

Arguments:
  SAR_OUT  sar output file in binary form.

EOF
}

main() {
  local sar_out="$1"
  local sadf_out="${sar_out%.*}.tsv"
  sadf_out=${sadf_out/sar/sadf}
  echo "$program: Write sadf output to $sadf_out"
  echo "host\tinterval\ttimestamp\tdevice\tfield\tvalue" > "$sadf_out"
  sadf -p -P 0 -U "$sar_out" >> "$sadf_out"
}

if [[ $# -eq 0 ]]; then
  usage
  exit 1
elif [[ "$1" = "-h" || "$1" = "--help" ]]; then
  usage
else
  main $@
fi
