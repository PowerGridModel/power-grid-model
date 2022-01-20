<!--
SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>

SPDX-License-Identifier: MPL-2.0
-->

# How to Contribute

We'd love to accept contributions to this project in any forms. 
You do not need to be a programming or power system expert to make a contribution.
There are just a few small guidelines you need to follow before making a change.

## Ways of contributing

Contribution does not necessarily mean committing code to the repository. 
We recognize different levels of contributions as shown below in increasing order of dedication:

1. Test and use the libray. Give feedback on the user experience or suggest new features.
2. Validate the model against other existing libraries. Provide validation test cases.
3. Report bugs.
4. Improve the Python interface or helper functions.
5. Contributing to the C++ core
    1. Develop new mathematical algorithms
    1. Improve or add new features into the C++ codebase

## Filing bugs and change requests

You can file bugs against and change request for the project via GitHub issues. Consult [GitHub Help](https://docs.github.com/en/free-pro-team@latest/github/managing-your-work-on-github/creating-an-issue) for more
information on using GitHub issues.

## Community Guidelines

This project follows the following [Code of Conduct](CODE_OF_CONDUCT.md).

## Style Guide

For both C++ and Python code we use the pipeline to automatically check the formatting.
We use `black` to check Python code and `clang-format` to check C++ code.
If the code format is not complying, the pipeline will fail the pull request will be blocked.

### Python

This project uses the PEP 8 Style Guide for Python Code. For all details about the various conventions please refer to:

[PEP 8](https://www.python.org/dev/peps/pep-0008)

Tip: Use [black](https://github.com/psf/black) to automatically format your Python code to conform to the PEP 8 style guide.

Furthermore, the following conventions apply:

* Maximum line length: 120 characters
* Double quotes for strings, keys etc.
    * Except when double quotes in the middle of a string are required.

### C++

This project uses Google Format Style (NOTE: not Google C++ Programming Style) to format the C++ code.

Tip: Use [clang-format](https://clang.llvm.org/docs/ClangFormat.html) to format your C++ code.

## REUSE Compliance

All the files in the repository need to be [REUSE compliant](https://reuse.software/). 
We use the pipeline to automatically check this.
If there are files which are not complying, the pipeline will fail the pull request will be blocked.


## Git branching

This project uses the [GitHub flow Workflow](https://guides.github.com/introduction/flow/) and branching model. 
The `main` branch always contains the latest release. 
New feature/fix branches are branched from `main`. 
When a feature/fix is finished it is merged back into `main` via a 
[Pull Request](https://docs.github.com/en/github/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/about-pull-requests).

In case of major version release with new features and/or breaking changes, we might temporarily create a 
`release/` branch to hold all the changes until they are merged into `main`.


## Signing the Developer Certificate of Origin (DCO)

This project utilize a Developer Certificate of Origin (DCO) to ensure that 
each commit was written by the author or that the author has the appropriate rights 
necessary to contribute the change. 
Specifically, we utilize [Developer Certificate of Origin, Version 1.1](http://developercertificate.org/),  
which is the same mechanism that the Linux® Kernel and many other communities use to manage code contributions. 
The DCO is considered one of the simplest tools for sign-offs from contributors as the representations are 
meant to be easy to read and indicating signoff is done as a part of the commit message.

This means that each commit must include a DCO which looks like this:

`Signed-off-by: Joe Smith <joe.smith@email.com>`

The project requires that the name used is your real name and the e-mail used is your real e-mail. 
Neither anonymous contributors nor those utilizing pseudonyms will be accepted.

There are other great tools out there to manage DCO signoffs for developers to make it much easier to do signoffs:
* Git makes it easy to add this line to your commit messages. Make sure the `user.name` and `user.email` are set in your git configs. Use `-s` or `--signoff` to add the Signed-off-by line to the end of the commit message.
* [GitHub UI integrations]( https://github.com/scottrigby/dco-gh-ui ) for adding the signoff automatically to commits made with the GitHub browser UI
* Additionally, it is possible to use shell scripting to automatically apply the sign-off. For an example for bash to be put into a .bashrc file, see [here](https://wiki.lfenergy.org/display/HOME/Contribution+and+Compliance+Guidelines+for+LF+Energy+Foundation+hosted+projects). 
* Alternatively, you can add `prepare-commit-msg hook` in .git/hooks directory. For an example, see [here](https://github.com/Samsung/ONE-vscode/wiki/ONE-vscode-Developer's-Certificate-of-Origin).

## Code reviews

All patches and contributions, including patches and contributions by project members, require review by one of the maintainers of the project. We
use GitHub pull requests for this purpose. Consult the pull request process below and the
[GitHub Help](https://help.github.com/articles/about-pull-requests/) for more
information on using pull requests


## Pull Request Process
Contributions should be submitted as Github pull requests. See [Creating a pull request](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request) if you're unfamiliar with this concept.

The process for a code change and pull request you should follow:

1. Create a topic branch in your local repository, following the naming format
"feature-###" or "fix-###". For more information see the Git branching guideline.
1. Make changes, compile, and test thoroughly. Ensure any install or build dependencies are removed before the end of the layer when doing a build. Code style should match existing style and conventions, and changes should be focused on the topic the pull request will be addressed. For more information see the style guide.
1. Push commits to your fork.
1. Create a Github pull request from your topic branch.
1. Pull requests will be reviewed by one of the maintainers who may discuss, offer constructive feedback, request changes, or approve
the work. For more information see the Code review guideline.
1. Upon receiving the sign-off of one of the maintainers you may merge your changes, or if you
   do not have permission to do that, you may request a maintainer to merge it for you.


## Attribution

This Contributing.md is adapted from Google
available at
https://github.com/google/new-project/blob/master/docs/contributing.md
