<!--
SPDX-FileCopyrightText: Contributors to the Power Grid Model project <powergridmodel@lfenergy.org>

SPDX-License-Identifier: MPL-2.0
-->

# Container Setup Guide

This document explains how you can utilize the container setup provided by us, to contribute to power-grid-model only on
the Python side.

## Setup Requirements

To begin using the container there are two open-source applications that must be installed.

* Podman (<https://podman.io/>) - a container engine that lets developers build, run, and manage containers and pods
without needing a background service.
* DevPod (<https://devpod.sh/>) - a tool that instantiates reproducible, disposable development environments allowing
developers to code inside the containers with their usual editors.

## The Recommended Setup Process

### Cloning The Repository

Firstly, setup the repository by cloning it into the desired destination.

```shell
git clone <https>
```

### DevPod Setup

Secondly, open DevPod and go into the "Providers" section. There click on "+ Add", select the docker icon, give it a
name and click on save. In the same window open advanced settings and under Docker path add the path to podman.exe (If
you are on Windows, it likely is in Program Files\RedHat directory).

### Podman Setup

Thirdly, to initialize Podman the Podman Desktop application has to be opened and "initialize and start" button has to
be pressed. If the start succeeds, there will be a keyword "RUNNING" in green displayed.

### Initializing the container

Finally, to initialize and open the container everything will be done through DevPod.

* Upon openning DevPod go to "Workspaces" and click on "Create Workspace".
* Under the "Enter Workspace Source", select the "Folder" option and navigate to the destination where you cloned the
repository to.
* Under the "Default IDE" select VSCode.
* Under "Provider" select the provider that you have setup previously (it should have the docker icon, but the path
should be to podman.exe)

Finally, click "Create Workspace"
