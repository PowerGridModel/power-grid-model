# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "power-grid-model"
copyright = "2022, alliander-opensource"
author = "alliander-opensource"
release = "1.4.1"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "myst_parser",
    "sphinx.ext.duration",
    "sphinx.ext.autosectionlabel",
    "sphinx.ext.autodoc",
]

templates_path = ["_templates"]


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "sphinx_rtd_theme"
html_static_path = ["_static"]

# Internationalization
# locale_dirs = ['locale/']   # path is example but recommended.
# gettext_compact = False     # optional.
