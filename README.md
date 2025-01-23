# VBAN-Text-Client
VBAN-TEXT send/reply management (minimal Win32 source code)

This source code for Win32 platforms is a minimal 'C' source code to manage VBAN-TEXT request and its possible response. With VB-Audio Matrix version x.0.1.2 the VBAN service can reply to question tag requests.

Example:
  - Command.Version = ?;
  - response: Command.Version = "VB-Audio Matrix (Version 1.0.1.2)";

# Compilation Instructions and Documentation
VBAN-TEXT Client is a Win32 application using native API and Windows socket that can be compiled for x32 and x64 target. Preprocessor and linket option are given in source header

# Licensing
All source code of this SDK is free to use in any kind of project interacting with VB-Audio Software. 

Copyright (c) 2025 Vincent Burel

# Links
- Download VB-Audio Matrix : www.vbaudiomatrix.com
- VBAN Protocol: https://vb-audio.com/Voicemeeter/vban.htm
- VB-Audio Support page: https://vb-audio.com/Services/support.htm
