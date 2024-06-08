mkdir Release
cd Release
if exist *.exe del *.exe
if exist *.dll del *.dll
cl /GL /O2 /Oi /MD ..\RGR\RGR.c /link /out:RGR.exe user32.lib gdi32.lib
cl /GL /O2 /Oi /MD ..\RGRDLL\RGRDLL.c /link /out:RGRDLL.dll /DLL
del *.obj *.lib *.exp
cd ..