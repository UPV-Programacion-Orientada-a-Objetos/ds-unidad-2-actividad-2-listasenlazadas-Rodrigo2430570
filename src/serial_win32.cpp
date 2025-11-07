#include "serial.h"
#include <stdio.h>
#include <string.h>

SerialPort::SerialPort()
#ifdef _WIN32
    : hComm(INVALID_HANDLE_VALUE)
#endif
    , opened(false) {}

SerialPort::~SerialPort() { close(); }

bool SerialPort::open(const char* comName, uint32_t baud) {
#ifdef _WIN32
    close();

    char path[64];
    // Formato \\.\COMx
    snprintf(path, sizeof(path), "\\\\.\\%s", comName);

    hComm = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hComm == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "[ERROR] No se pudo abrir %s.\n", path);
        opened = false;
        return false;
    }

    // Configurar buffers
    SetupComm(hComm, 4096, 4096);

    // Configurar DCB
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(hComm, &dcb)) {
        fprintf(stderr, "[ERROR] GetCommState fallo.\n");
        close();
        return false;
    }

    dcb.BaudRate = baud;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    if (!SetCommState(hComm, &dcb)) {
        fprintf(stderr, "[ERROR] SetCommState fallo.\n");
        close();
        return false;
    }

    // timeouts (lectura por linea con timeout)
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 50;          // ms entre bytes
    timeouts.ReadTotalTimeoutMultiplier = 0;    
    timeouts.ReadTotalTimeoutConstant = 50;     // constante base
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;
    if (!SetCommTimeouts(hComm, &timeouts)) {
        fprintf(stderr, "[ERROR] SetCommTimeouts fallo.\n");
        close();
        return false;
    }

    PurgeComm(hComm, PURGE_RXCLEAR | PURGE_TXCLEAR);

    opened = true;
    return true;
#else
    (void)comName; (void)baud;
    fprintf(stderr, "Serial solo implementado en Windows.\n");
    return false;
#endif
}

int SerialPort::readLine(char* buffer, int cap, uint32_t timeoutMs) {
#ifdef _WIN32
    if (!opened || hComm == INVALID_HANDLE_VALUE) return -1;
    if (cap <= 1) return -1;

    DWORD startTick = GetTickCount();
    int idx = 0;

    while (1) {
        // timeout global
        DWORD now = GetTickCount();
        if (timeoutMs > 0 && (now - startTick) >= timeoutMs) {
            if (idx == 0) return 0; // timeout sin datos
            break; // devolver lo leido
        }

        char ch;
        DWORD read = 0;
        BOOL ok = ReadFile(hComm, &ch, 1, &read, NULL);
        if (!ok) {
            fprintf(stderr, "[ERROR] ReadFile fallo.\n");
            return -1;
        }
        if (read == 0) {
            // sin datos por ahora, dormir un poco
            Sleep(5);
            continue;
        }

        if (ch == '\r') continue; // ignorar CR
        if (ch == '\n') break;    // fin de linea

        if (idx < cap - 1) {
            buffer[idx++] = ch;
        }
    }

    buffer[idx] = '\0';
    return idx;
#else
    (void)buffer; (void)cap; (void)timeoutMs;
    return -1;
#endif
}

void SerialPort::close() {
#ifdef _WIN32
    if (hComm != INVALID_HANDLE_VALUE) {
        CloseHandle(hComm);
        hComm = INVALID_HANDLE_VALUE;
    }
#endif
    opened = false;
}
