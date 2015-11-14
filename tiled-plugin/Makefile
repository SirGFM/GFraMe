
#===============================================================================
# Stupid Makefile to  copy and compile the plugin with Tiled; It requires that
# the tiled code is checked out somewhere in your computer
#===============================================================================

#===============================================================================
# Check that the tiled source dir was set
#-------------------------------------------------------------------------------
ifndef TILED_DIR
  $(info Must specify the directory where Tiled was checked out through the \
         TILED_DIR variable;)
  $(info option 1: '$$ make TILED_DIR=dir')
  $(info option 2: '$$ exporte TILED_DIR=dir' (only once))
  $(info .         '$$ make' (any number of times, afterward))
  $(error NO TILED_DIR SPECIFIED)
endif
#===============================================================================

#===============================================================================
# Get the exporter dir
#-------------------------------------------------------------------------------
DST_DIR := $(TILED_DIR)/src/plugins/gfmExporter
#===============================================================================

all:
	@# Update the plugin's sources (on the tiled dir)
	@mkdir -p $(DST_DIR)
	@cp -f plugin.json gfmExporter* $(DST_DIR)
	@# Regenerate the Makefile
	@cd $(DST_DIR); qmake
	@# Compile it
	@make -C $(DST_DIR)

