#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import os
from pathlib import Path
import subprocess

rtd_build = os.environ.get("READTHEDOCS", None) == "True"

if rtd_build:
    subprocess.call("doxygen --version; doxygen doxygen.conf", shell=True)

# Set paths for python modules (for autodoc). The paths must be absolute.
py_root_path = Path(".") / ".." / "src" / "python"
py_root_path = py_root_path.absolute()
sys.path.insert(0, str(py_root_path))

extensions = [
    "breathe",
    "sphinxarg.ext",
    "sphinx.ext.napoleon",
    "sphinx.ext.autodoc",
    "sphinx.ext.intersphinx"
]

templates_path = ["_templates"]

source_suffix = {".rst": "restructuredtext"}

master_doc = "index"

# General information about the project.
project = "Orcus"
copyright = "2026, Kohei Yoshida"

version = "0.22"
release = "0.22.0"

exclude_patterns = ["_build"]

pygments_style = "sphinx"
html_theme = "sphinx_rtd_theme"

# theme-specific options
html_theme_options = {
    "navigation_depth": 2,
    "prev_next_buttons_location": "both",
    "style_external_links": False,
}

html_static_path = ["_static"]

htmlhelp_basename = "orcusdoc"

latex_elements = {}

latex_documents = [
  ("index", "orcus.tex", "Orcus Documentation",
   "Kohei Yoshida", "manual"),
]

man_pages = [
    ("index", "orcus", "Orcus Documentation",
     ["Kohei Yoshida"], 1)
]

texinfo_documents = [
  ("index", "orcus", "Orcus Documentation",
   "Kohei Yoshida", "Orcus", "One line description of project.",
   "Miscellaneous"),
]

breathe_projects = {"orcus": "./_doxygen/xml"}

breathe_default_project = "orcus"

breathe_default_members = ("members", "undoc-members")

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
    "ixion": ("https://ixion.readthedocs.io/en/latest", None),
}

autodoc_member_order = "bysource"
