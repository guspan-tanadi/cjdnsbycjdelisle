#!/usr/bin/env bash

set -euf -o pipefail

CONF_DIR="/etc/cjdns"

if [ ! -f "$CONF_DIR/cjdroute.conf" ]; then
  echo "generate $CONF_DIR/cjdroute.conf"
  conf="$(cjdroute --genconf | cjdroute --cleanconf)"
  echo "$conf" > "$CONF_DIR/cjdroute.conf"
fi

cjdroute --nobg < "$CONF_DIR/cjdroute.conf" || exit $?
exit 0
