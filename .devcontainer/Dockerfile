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
  # build-essential ninja-build cmake
  g++ git ca-certificates \
  # removes the index files from apt-get update step  
  && rm -rf /var/lib/apt/lists/*

# recognize the pgm directory as safe
RUN git config --system --add safe.directory /power-grid-model

RUN python -m pip install --upgrade pip

COPY . .

RUN python -m pip install -e .[dev]

# Create a directory for the .so where we look for it, link the current .so location to the one that we look at.
RUN mkdir -p src/power_grid_model/_core/power_grid_model_c/lib && \
    ln -sf /usr/local/lib/python3.13/site-packages/power_grid_model/_core/power_grid_model_c/lib/libpower_grid_model_c.so \
          src/power_grid_model/_core/power_grid_model_c/lib/
