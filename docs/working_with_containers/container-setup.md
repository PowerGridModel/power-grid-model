<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Container Setup Guide

This document explains how you can utilize the container setup provided by us, to contribute to power-grid-model only on
the Python side.

## Setup Requirements

To begin using the container, you have several options, e.g.:

* Podman + DevPod (both are free, open-source tools)
* Docker + DevPod (NOTE: Docker Desktop requires a subscription that is not free for commercial usage)

* Podman (<https://podman.io/>) - a container engine that lets developers build, run, and manage containers and pods
without needing a background service.
* DevPod (<https://devpod.sh/>) - a tool that instantiates reproducible, disposable development environments allowing
developers to code inside the containers with their usual editors.
* Docker (<https://www.docker.com/>) - a popular container platform that uses a background service (daemon) to build,
run, and manage containerized applications. (NOTE: Docker Desktop requires a subscription that is not free for
commercial usage).

## The Recommended Setup Process

### Cloning The Repository

Firstly, setup the repository by cloning it into the desired destination.

```shell
git clone <https>
```

Make sure that you are on the right branch.

### Podman and DevPod Setup

#### DevPod Setup

Open DevPod and go into the "Providers" section. There click on "+ Add", select the docker icon, give it a
name and click on save. In the same window open advanced settings and under Docker path add the path to 'podman'
executable (If you are on Windows, it likely is in Program Files\RedHat directory).

#### Podman Setup

To initialize Podman the Podman Desktop application has to be opened and "initialize and start" button has to
be pressed. If the start succeeds, there will be a keyword "RUNNING" in green displayed.

#### Initializing the container

Finally, to initialize and open the container everything will be done through DevPod.

* Upon openning DevPod go to "Workspaces" and click on "Create Workspace".
* Under the "Enter Workspace Source", select the "Folder" option and navigate to the destination where you cloned the
repository to.
* Under the "Default IDE" select VSCode.
* Under "Provider" select the provider that you have setup previously (it should have the docker icon)

Finally, click "Create Workspace"

### Docker and DevPod Setup

#### Docker Setup

Before you begin, install Docker Desktop and open it. It will initialize automatically when launched.

#### DevPod Setup

Secondly, open DevPod and go into the "Providers" section. There click on "+ Add", select the docker icon, give it a
name and click on save. DevPod should find the PATH to the 'docker' executable itself, however if you think that there
are issues within your setup you can adjust the PATH in the advanced settings by adding the path to the 'docker'
executable under Docker path.

For the final initialization step, see [Initializing the container](#initializing-the-container).

## Possible solutions to issues during setup

* If you are facing issues while building the container then try and allocate more memory in Podman. Go to
Podman -> Settings (Bottom left corner) -> Resources -> Edit.
* Try and make a fresh clone of the repository and redo the setup on it.

## Support for Apple Container

We have tested a container setup with Apple's container app. However, currently there is not a simplistic way to open
that container within VSCode. With future plugin developments we expect to add support for Apple's container.
