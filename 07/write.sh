#!/bin/bash

# Tableau contenant les différentes valeurs
messages=("haha" "hihi" "hoho" "huhu")

# Boucle infinie
while true; do
    for message in "${messages[@]}"; do
        echo "$message" > /sys/kernel/debug/fourtytwo/foo
    done
done

