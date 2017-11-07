# Makefile to build class 'swisseph' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# input source file (class name == source file basename)
class.sources = swisseph~.c

# all extra files to be included in binary distribution of the library
datafiles = swisseph~-help.pd swisseph~-meta.pd README.md

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
