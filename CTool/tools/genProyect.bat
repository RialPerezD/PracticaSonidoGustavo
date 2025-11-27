@echo off
@cls

echo ==============================================
echo Generando solucion de Visual Studio...
echo ==============================================

:: Obtener ruta del proyecto (dos niveles arriba de \tools)
set PROJECT_DIR=%~dp0..
set BUILD_DIR=%PROJECT_DIR%\build

:: Crear carpeta build si no existe
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Ir a build
cd /d "%BUILD_DIR%"

:: Ejecutar CMake para generar solución VS2022
cmake "%PROJECT_DIR%" -G "Visual Studio 17 2022"

echo ==============================================
echo   Solucion generada dentro de /build
echo   Archivo: PracticaSonidoGustavo.sln
echo ==============================================

pause
