# Doc

## Prerequisite

- python 3
- doxygen

```sh

pip install sphinx pydata-sphinx-theme breathe

```

## Build

```sh

doxygen ./doxygen/Doxyfile

sphinx-build -b html -E sphinx/source build_sphinx -D"breathe_projects.fitoria=../../build_doxygen/xml"

```
