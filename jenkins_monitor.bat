tasklist /FI "IMAGENAME eq USBCMDAP.exe" 2>NUL | find /I /N "USBCMDAP.exe">NUL
if "%ERRORLEVEL%"=="0" taskkill /F /IM USBCMDAP.exe
:: TASKKILL /F /IM "USBCMDAP.exe"
C:\wamp\bin\php\php5.4.3\php.exe ..\..\buildlights.php
Exit