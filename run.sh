#!/bin/bash

# Скрипт для запуска приложения

APP_PATH="build/DutyScheduleApp"

if [ ! -f "$APP_PATH" ]; then
    echo "❌ Исполняемый файл не найден: $APP_PATH"
    echo "💡 Сначала соберите проект: ./build.sh"
    exit 1
fi

echo "🚀 Запуск сервера..."
echo "📱 Откройте браузер и перейдите по адресу: http://localhost:8080"
echo "⏹️  Для остановки нажмите Ctrl+C"
echo ""

cd "$(dirname "$0")"
./build/DutyScheduleApp

