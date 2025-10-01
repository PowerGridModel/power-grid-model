# SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>
#
# SPDX-License-Identifier: MPL-2.0

FROM python:3.13-slim

# Prevents Python from writing pyc files.
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

WORKDIR /power-grid-model

# update the packages before installing, always answer with yes, do not install any recommendations by the installed packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    git ca-certificates \
  # removes the index files from apt-get update step  
  && rm -rf /var/lib/apt/lists/*
# test change
# recognize the pgm directory as safe
RUN git config --system --add safe.directory /power-grid-model

# container build asks for a venv
ENV VIRTUAL_ENV=/opt/venv
RUN python -m venv "$VIRTUAL_ENV"
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

RUN python -m pip install --upgrade pip \
 && python -m pip install "power-grid-model[dev]"

COPY . .
