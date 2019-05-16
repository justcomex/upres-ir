if exist *.exe del *.exe
if exist *.dll del *.dll
cl /EHsc driver.c kernel32.lib user32.lib gdi32.lib advapi32.lib
cl /LD info.c kernel32.lib user32.lib gdi32.lib advapi32.lib
del *.obj *.lib *.exp
start driver.exe