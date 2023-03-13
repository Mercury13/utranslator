@rem Settings
@set VERSION=0.3.1
@set QTDIR=c:\Qt\6.1.3\mingw81_64
@rem Workaround for that Qt, maybe QtcW7Compat.7z
@rem set WORKAROUND=
@set MINGW=c:\msys64\mingw64\bin
@set SEVENZIP="c:\Program Files\7-zip\7z.exe"

@rem Rest things
@set PRONAME=UTranslator\UTranslator.pro
@set PRONAME_CON=UTransCon\UTransCon.pro
@set EXENAME=UTranslator.exe
@set EXENAME_CON=UTransCon.exe
@set ARCNAME=UTranslator-w64-%VERSION%.7z
@set BUILD=~Build-win64
@set BUILD_CON=~Build-CON-win64
@set DEPLOY=~Deploy
@set DEPLOY1=~Deployed

@path %MINGW%;%PATH%

@echo ===== Checking for tools =====

@if exist %SEVENZIP% goto sz_ok
@echo BAD: 7-zip not found. Install it into standard directory, or change SEVENZIP variable.
@goto end
:sz_ok
@echo 7-zip OK

@if exist %QTDIR%\bin\qmake.exe goto qt_ok
@echo BAD: Qmake not found. Install Qt, and set QTDIR variable.
@goto end
:qt_ok
@echo Qt OK

@if exist %MINGW%\g++.exe goto mingw_ok
@echo BAD: MinGW not found. Install it into standard MSYS directory.
@goto end
:mingw_ok
@echo MinGW OK

@echo ===== Creating directories =====
@if exist %DEPLOY% del /S /Q %DEPLOY% >nul
@if exist %DEPLOY% rmdir /S /Q %DEPLOY% >nul
@if not exist %DEPLOY% md %DEPLOY%
@if not exist %DEPLOY1% md %DEPLOY1%
@if not exist %BUILD_CON% md %BUILD_CON%
@if not exist %BUILD% md %BUILD%

@echo.
@echo ===== Building Console =====
@cd %BUILD_CON%
@%QTDIR%\bin\qmake.exe ..\%PRONAME_CON% -r -spec win32-g++ "CONFIG+=release"
@%MINGW%\mingw32-make.exe -f Makefile.Release -j%NUMBER_OF_PROCESSORS%
@cd ..

@if not exist %BUILD_CON%\release\%EXENAME_CON% goto no_exe

@echo.
@echo ===== Building UTranslator =====
@cd %BUILD%
@%QTDIR%\bin\qmake.exe ..\%PRONAME% -r -spec win32-g++ "CONFIG+=release"
@%MINGW%\mingw32-make.exe -f Makefile.Release -j%NUMBER_OF_PROCESSORS%
@cd ..

@if not exist %BUILD%\release\%EXENAME% goto no_exe

@echo.
@echo ===== Copying files =====
@copy %BUILD%\release\%EXENAME% %DEPLOY%
@copy %BUILD_CON%\release\%EXENAME_CON% %DEPLOY%
@copy %MINGW%\libgcc_s_seh-1.dll %DEPLOY%
@copy "%MINGW%\libstdc++-6.dll" %DEPLOY%
@copy %MINGW%\libwinpthread-1.dll %DEPLOY%
@copy %QTDIR%\bin\Qt6Core.dll %DEPLOY%
@copy %QTDIR%\bin\Qt6Gui.dll %DEPLOY%
@copy %QTDIR%\bin\Qt6Widgets.dll %DEPLOY%
@copy %QTDIR%\bin\Qt6Svg.dll %DEPLOY%
@copy %QTDIR%\bin\Qt6SvgWidgets.dll %DEPLOY%
@copy MiscFiles\UTranslator.xml %DEPLOY%
@copy LICENSE %DEPLOY%
@md %DEPLOY%\platforms
@copy %QTDIR%\plugins\platforms\qwindows.dll %DEPLOY%\platforms
@md %DEPLOY%\styles
@copy %QTDIR%\plugins\styles\qwindowsvistastyle.dll %DEPLOY%\styles
@if [%WORKAROUND%] == [] goto no_workaround
@%SEVENZIP% x MiscFiles\%WORKAROUND% -o%DEPLOY%
:no_workaround

@REM @echo.
@REM @echo ===== Building L10n =====
@REM @md %DEPLOY%\Languages
@REM @rem Russian
@REM @set DIR_RU=%DEPLOY%\Languages\Russian
@REM @md %DIR_RU%
@REM @copy lang-src\ru\locale.xml %DIR_RU%
@REM @%UTRANSL% lang-src\ru.uorig -build:%DIR_RU%
@REM @copy %QTDIR%\translations\qtbase_ru.qm %DIR_RU%
@REM @rem English
@REM @set DIR_EN=%DEPLOY%\Languages\English
@REM @md %DIR_EN%
@REM @copy lang-src\en\locale.xml %DIR_EN%
@REM @%UTRANSL% lang-src\en.utran -build:%DIR_EN%
@REM @rem no QM for English

@echo.
@echo ===== Archiving =====
@cd %DEPLOY%
@set ARCPATH=..\%DEPLOY1%\%ARCNAME%
@if exist %ARCPATH% del %ARCPATH%
@%SEVENZIP% a %ARCPATH% * -mx9 -mmt%NUMBER_OF_PROCESSORS%
@cd ..

@goto end

:no_exe
@echo BAD: EXE NOT FOUND
@goto end

:end
@pause