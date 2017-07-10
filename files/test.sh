#!/usr/bin/env bash
echo "HTTP/1.0 200 OK"
echo "Content-Type: text/html"
echo
cat <<%ENDE%
<H1>Hallo Genossen!</H1>

Es ist <b>$(date)</b>, also schon spaet.

Sie sind verbunden mit $(hostname), der momentan
$(uptime) viel Last schiebt. Schnief.

PID ist $$

%ENDE%
