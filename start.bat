@echo off
cd /d C:\dev\aicoach\build
set LLM_API_URL=http://localhost:11434/v1/chat/completions
set LLM_API_KEY=ollama
set LLM_MODEL=llama3.1:latest
set DATA_DIR=C:\dev\aicoach\data
set FRONTEND_DIR=C:\dev\aicoach\frontend
echo ========================================
echo   AI Coach
echo   http://localhost:8080/
echo ========================================
echo Make sure Ollama is running (ollama serve)
echo.
aicoach.exe
pause