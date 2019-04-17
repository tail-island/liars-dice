@echo off

msbuild /p:Configuration=Release;Platform=x64

copy x64\release\liars-dice.exe dist\win-x64\

copy x64\release\fool.exe       dist\win-x64\fool\
copy x64\release\hardhead.exe   dist\win-x64\hardhead\
copy x64\release\optimist.exe   dist\win-x64\optimist\
copy x64\release\pessimist.exe  dist\win-x64\pessimist\
copy x64\release\timid.exe      dist\win-x64\timid\
