#!/bin/bash

# The Docker image used to generate the documentation uses the latest
# Percona Proxy image. If the PERCONA_PROXY_IMAGE environment variable is defined,
# the image name from that is used instead.

python3 -m venv venv
source venv/bin/activate
pip install mariadb jinja2 requests
python3 generate.py
deactivate
rm -r venv
