# This make include file defines env variables required to build XDC
XDCBUILDCFG = $(CONFIG_BLD_XDC_$(ISA))
export XDCBUILDCFG
# Nothing beyond this point
