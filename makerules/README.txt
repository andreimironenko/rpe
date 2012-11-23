########################################################
# Read this file first 
########################################################

Quick help to build:

1. open a command window. 
2. Change directory to the top level folder packages (or avmecomp)
3. Set environment variabled as below:
  - set CYGWINPATH=<cygwin-install-directory>    eg., set CYGWINPATH=e:/cygwin
  - set ROOTDIR=<$OMXINSTALLDIR>                 eg., set ROOTDIR=p:/avmecom
  - set PATH=%CYGWINPATH%/bin;
  
  Here ROOTDIR is the location where "makerules" directory can be found
  
4. Run the following make command to build VC3 
  - make vc3ducati EXTERNAL_SW_ROOT=<$TISWINSTALLDIR> INTERNAL_SW_ROOT=<$OMXINSTALLDIR> 
    DEST_ROOT=<object/executable destination directory> PLATFORM=ti816x-evm
  
  This overrides the settings in makerules/env.mk and makerules/build_config.mk.
  The above command build vc3 executable binaries for ti816x-evm (Netra) platform.

NOTE: The builds have been tested with Cygwin 1.7.3-1

