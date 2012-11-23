function getMemSegmentDefinition(platFormMem)
{
  var memory = new Array();

  memory[0] = ["VIDEO_M3_VECTOR",
  {
          name: "VIDEO_M3_VECTOR",
          base: 0x0,
          len:  0x400,
          space: "code"
  }];

  memory[1] = ["VPSS_M3_VECTOR",
  {
          name: "VPSS_M3_VECTOR",
          base: 0x400,
          len:  0x600,
          space: "code"
  }];

  memory[2] = ["DSP",
  {
          name: "DSP",
          base: 0x99500000,
          len:  0x00C00000,
          space: "code/data"
  }];

  memory[3] = ["VIDEO_M3_EVENT_BUFFER",
  {
          name: "VIDEO_M3_EVENT_BUFFER",
          base: 0x9D500000,
          len:  0x00100000,
          space: "data"
  }];

  memory[4] = ["VIDEO_M3_DATA",
  {
          name: "VIDEO_M3_DATA",
          base: 0x9D600000,
          len:  0x00D00000,
          space: "data"
  }];

  memory[5] = ["VIDEO_M3_CODE",
  {
          name: "VIDEO_M3_CODE",
          base: 0x9E300000,
          len:  0x00100000,
          space: "code"
  }];

  memory[6] = ["LOGGER_SM",
  {
          name: "LOGGER_SM",
          base: 0x9E400000,
          len:  0x00200000,
          space: "data"
  }];

  memory[7] = ["VPSS_M3_EVENT_BUFFER",
  {
          name: "VPSS_M3_EVENT_BUFFER",
          base: 0x9E600000,
          len:  0x00100000,
          space: "data"
  }];

  memory[8] = ["VPSS_M3_DATA",
  {
          name: "VPSS_M3_DATA",
          base: 0x9E700000,
          len:  0x00E00000,
          space: "data"
  }];

  memory[9] = ["VPSS_M3_CODE",
  {
          name: "VPSS_M3_CODE",
          base: 0x9F500000,
          len:  0x00200000,
          space: "code"
  }];


  memory[10] = ["IPC_SR_COMMON",
  {
          name: "IPC_SR_COMMON",
          base: 0x9F700000,
          len:  0x00200000,
          space: "data"
  }];

  memory[11] = ["HDVPSS_NOTIFY_MEM",
  {
          name: "HDVPSS_NOTIFY_MEM",
          base: 0xBF900000,
          len:  0x00200000,
          space: "data"
  }];

  memory[12] = ["HDVPSS_V4L2_FBDEF_MEM",
  {
          name: "HDVPSS_V4L2_FBDEF_MEM",
          base: 0xBFB00000,
          len:  0x00200000,
          space: "data"
  }];

  memory[13] = ["HDVPSS_DESC",
  {
          name: "HDVPSS_DESC",
          base: 0xBFD00000,
          len:  0x00200000,
          space: "data"
  }];

  memory[14] = ["MEMCFG_SPACE",
  {
          name: "MEMCFG_SPACE",
          base: 0xBFF00000,
          len:  0x000FEFFC,
          space: "data"
  }];
  
  memory[15] = ["STOPFIRMWARE_FLAG",
  {
          name: "STOPFIRMWARE_FLAG",
          base: 0xBFFFEFFC,
          len:  0x00000004,
          space: "data"
  }];

  memory[16] = ["VIDEO_M3_LOAD_TABLE",
  {
          name: "VIDEO_M3_LOAD_TABLE",
          base: 0xBFFFF000,
          len:  0x00000800,
          space: "data"
  }];

  memory[17] = ["VPSS_M3_LOAD_TABLE",
  {
          name: "VPSS_M3_LOAD_TABLE",
          base: 0xBFFFF800,
          len:  0x00000800,
          space: "data"
  }];

  return (memory);
}

function getMemSegmentDefinition1(platFormMem)
{
  var memory = new Array();

  memory[0] = ["DSP",
  {
          name: "DSP",
          base: 0x96C00000,
          len:  0x02000000,
          space: "code/data"
  }];

  memory[1] = ["DUCATI_M3_1 ",
  {
          name: "DUCATI_M3_1",
          base: 0x9A000000,
          len:  0x00F00000,
          space: "data"
  }];

  memory[2] = ["DUCATI_M3_0",
  {
          name: "DUCATI_M3_0",
          base: 0x9AF00000,
          len:  0x00F00000,
          space: "data"
  }];

  memory[3] = ["IPC_SR_COMMON",
  {
          name: "IPC_SR_COMMON",
          base: 0x9BE00000,
          len:  0x00200000,
          space: "data"
  }];

  memory[4] = ["MC_HDVPSS_NOTIFY_MEM",
  {
          name: "MC_HDVPSS_NOTIFY_MEM",
          base: 0x9F900000,
          len:  0x00200000,
          space: "data"
  }];

  memory[5] = ["MC_HDVPSS_V4L2_FBDEF_MEM ",
  {
          name: "MC_HDVPSS_V4L2_FBDEF_MEM ",
          base: 0x9FB00000,
          len:  0x00200000,
          space: "data"
  }];

  memory[6] = ["MC_HDVPSS_DESC",
  {
          name: "MC_HDVPSS_DESC",
          base: 0x9FD00000,
          len:  0x00200000,
          space: "data"
  }];

  memory[7] = ["MEMCFG_SPACE",
  {
          name: "MEMCFG_SPACE",
          base: 0x9FF00000,
          len:  0x000FFFFC,
          space: "data"
  }];
  
  memory[8] = ["STOPFIRMWARE_FLAG",
  {
          name: "STOPFIRMWARE_FLAG",
          base: 0x9FFFFFFC,
          len:  0x00000004,
          space: "data"
  }];

  return (memory);
}


var soc = java.lang.System.getenv("SOC");
print ("Soc Selected: " + soc);
if (soc == 'ti814x') {
 var device = 'TMS320DM8148';
 Build.platformTable['ti.platforms.evmDM8148'] =
  {
    l1DMode:"32k",
    l1PMode:"32k",
    l2Mode:"128k",            
    externalMemoryMap: getMemSegmentDefinition(platFormMem),
    codeMemory:"DSP",
    dataMemory:"DSP",
    stackMemory:"DSP"
  }; 
 
}
else if (soc == 'ti816x') {
 var device = 'TMS320DM8168';
 Build.platformTable['ti.platforms.evmDM8168'] =
 {
    l1DMode:"32k",
    l1PMode:"32k",
    l2Mode:"128k",            
    externalMemoryMap: getMemSegmentDefinition(platFormMem),
    codeMemory:"DSP",
    dataMemory:"DSP",
    stackMemory:"DSP"
 };
}
else if (soc == 'ti811x') {
 var device = 'TMS320TI811X';
 Build.platformTable['ti.platforms.evmTI811X'] =
  {
    l1DMode:"32k",
    l1PMode:"32k",
    l2Mode:"128k",            
    externalMemoryMap: getMemSegmentDefinition1(platFormMem),
    codeMemory:"DSP",
    dataMemory:"DSP",
    stackMemory:"DSP"
  }; 
 
}
else {
throw new Error("Unsupported value for SOC : " + soc);
}


