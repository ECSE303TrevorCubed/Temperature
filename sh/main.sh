#!/usr/bin/env bash

LED_PIN=25
WAIT_TIME=0.5

trap cleanup INT

function cleanup() {
  gpio mode $LED_PIN input
  exit 0
}

gpio mode $LED_PIN output

while true; do
  gpio write $LED_PIN 1
  sleep $WAIT_TIME
  gpio write $LED_PIN 0
  sleep $WAIT_TIME
done

cleanup
