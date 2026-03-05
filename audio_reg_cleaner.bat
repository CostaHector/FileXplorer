@echo off
echo Cleaning Windows audio registry entries for FileXplorer... Windows Audio Session API (WASAPI) Audio Policy Configuration
echo.

reg delete "HKEY_CURRENT_USER\Software\Microsoft\Internet Explorer\LowRegistry\Audio\PolicyConfig\PropertyStore" /f

echo Done! Please restart your application.
pause