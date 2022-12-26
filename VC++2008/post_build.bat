@echo off
rem %1 = $(SolutionDir) %2 = $(ConfigurationName)
xcopy /s /c /i /q /y "%1"\..\lang\*.* "%1"%2\lang\
copy /b /y "%1"\..\font\*.dat "%1"%2\
