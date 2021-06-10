echo on

xcopy /E PublicDecompWTMakefiles PublicDecompWT
call "C:\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64

nmake /f makefile.vc MSVC_VER=1910
nmake /f makefile.vc MSVC_VER=1910 dll

IF "%GDAL_DRIVER_PATH%"=="" SET GDAL_DRIVER_PATH=%PREFIX%\Library\lib\gdalplugins
IF not exist %GDAL_DRIVER_PATH% mkdir %GDAL_DRIVER_PATH%
copy gdal_MSG.dll %GDAL_DRIVER_PATH%

set ACTIVATE_DIR=%PREFIX%\etc\conda\activate.d
set DEACTIVATE_DIR=%PREFIX%\etc\conda\deactivate.d
IF not exist %ACTIVATE_DIR% mkdir %ACTIVATE_DIR%
IF not exist %DEACTIVATE_DIR% mkdir %DEACTIVATE_DIR%

copy %RECIPE_DIR%\install_msg_gdal_driver_lib.bat %ACTIVATE_DIR%\install_msg_gdal_driver_lib.bat
copy %RECIPE_DIR%\uninstall_msg_gdal_driver_lib.bat %DEACTIVATE_DIR%\uninstall_msg_gdal_driver_lib.bat

