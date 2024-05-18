# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "fitoria"
copyright = "2024, Ramirisu"
author = "Ramirisu"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ["breathe"]

breathe_default_project = "fitoria"

templates_path = ["_templates"]
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "pydata_sphinx_theme"
html_static_path = ["_static"]
html_css_files = ["css/custom.css"]
html_title = "fitoria"

html_theme_options = {
    "icon_links": [
        {
            "name": "GitHub",
            "url": "https://github.com/Ramirisu/fitoria",
            "icon": "fa-brands fa-github",
            "type": "fontawesome",
        }
    ],
}
