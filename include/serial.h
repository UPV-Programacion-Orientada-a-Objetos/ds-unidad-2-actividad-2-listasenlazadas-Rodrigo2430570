#ifndef SERIAL_H
#define SERIAL_H

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdint.h>

/**
 * \brief Pequeño envoltorio para puerto serial en Windows con API Win32.
 * No usa std::string ni STL.
 */
class SerialPort {
#ifdef _WIN32
    HANDLE hComm;
#endif
    bool opened;

public:
    SerialPort();
    ~SerialPort();

    // Abre \\.\COMx con el baud rate indicado. Devuelve true si OK.
    bool open(const char* comName, uint32_t baud);

    // Lee una línea (terminada en \n o \r) hacia buffer (capacidad cap).
    // Coloca terminador NUL. Devuelve cantidad de bytes útiles leídos (>=0),
    // o -1 en error, 0 en timeout sin datos.
    int readLine(char* buffer, int cap, uint32_t timeoutMs);

    void close();

    bool isOpen() const { return opened; }
};

#endif // SERIAL_H
