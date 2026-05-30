@rem Paths for UTranslator
@rem NOTE: You can make your own unversioned ~setup_local.bat.
@rem Scenarios:
@rem 1. 7-Zip is banned for itself, but it is a so indispensable
@rem    program that it it exists somewhere (irony).
@rem 2. Some program is installed as portable.
@rem 3. You use the newer Qt.

@set QTDIR=c:\Qt\6.2.4\mingw_64
@set MINGW=c:\msys64\mingw64\bin
@set SEVENZIP="c:\Program Files\7-zip\7z.exe"

@rem As 7-zip is banned sometimes, but it's ubiquitous...
@if exist %SEVENZIP% goto szexists

@set SEVENZIP1="C:\Program Files\PeaZip\res\bin\7z\7z.exe"
@if exist %SEVENZIP1% set SEVENZIP=%SEVENZIP1%

:szexists