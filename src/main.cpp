#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "serial.h"
#include "listas.h"
#include "tramas.h"

static void toUpperInPlace(char* s) {
    for (; *s; ++s) {
        *s = (char)toupper((unsigned char)*s);
    }
}

static void trim(char* s) {
    // derecha
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t')) { s[--len] = '\0'; }
    // izquierda
    int i = 0; while (s[i] == ' ' || s[i] == '\t') ++i; if (i > 0) memmove(s, s+i, (size_t)(len - i + 1));
}

struct Parsed {
    char tipo;  // 'L' o 'M'
    char ch;    // para L
    int  num;   // para M
    int  valid; // 1 si válido
};

static struct Parsed parse_line(const char* line) {
    struct Parsed p; p.tipo='\0'; p.ch='\0'; p.num=0; p.valid=0;
    if (!line) return p;

    char buf[64];
    strncpy(buf, line, sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';

    // remover espacios
    trim(buf);

    char* tok1 = strtok(buf, ",");
    char* tok2 = strtok(NULL, ",");
    if (!tok1 || !tok2) return p;

    if (tok1[0] == 'L' || tok1[0] == 'l') {
        // convertir palabra Space a espacio real (case-insensitive)
        char upperTok2[32];
        strncpy(upperTok2, tok2, sizeof(upperTok2)-1);
        upperTok2[sizeof(upperTok2)-1] = '\0';
        toUpperInPlace(upperTok2);
        if (strcmp(upperTok2, "SPACE") == 0) {
            p.tipo = 'L'; p.ch = ' '; p.valid = 1; return p;
        }
        // si es una sola letra
        if (tok2[0] && !tok2[1]) {
            char c = tok2[0];
            // mantener signo/puntuaciones; mapear minuscula a mayus
            if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
            p.tipo = 'L'; p.ch = c; p.valid = 1; return p;
        }
        return p;
    }
    if (tok1[0] == 'M' || tok1[0] == 'm') {
        // permitir +/- numero
        int sign = 1; int i = 0;
        if (tok2[0] == '-') { sign = -1; i = 1; }
        else if (tok2[0] == '+') { sign = 1; i = 1; }
        int val = 0; int any = 0;
        for (; tok2[i]; ++i) {
            if (tok2[i] >= '0' && tok2[i] <= '9') { val = val*10 + (tok2[i]-'0'); any = 1; }
            else { any = 0; break; }
        }
        if (any) {
            p.tipo='M'; p.num = sign*val; p.valid=1; return p;
        }
        return p;
    }

    return p;
}

static void printMensajeCuadrado(const ListaDeCarga* lista) {
    // Imprime como en el ejemplo: [H][O]...
    // Necesitamos iterar sin exponer internals; hacemos copia mínima: this file friend-like
    // Redefinir aquí las estructuras no es deseable; imprimimos directamente recorriendo (hack):
    // Nota: para mantener encapsulamiento, ListaDeCarga ofrece imprimirMensaje() final sin brackets.
    // Aquí mostramos una versión con brackets usando un truco: no disponible; usamos imprimirMensaje simple en logs de avance.
    (void)lista; // no usado; mantenido por compatibilidad
}

int main(int argc, char** argv) {
    printf("Iniciando Decodificador PRT-7.\n");

    const char* com = "COM3"; // por defecto
    unsigned int baud = 9600;
    int maxLines = -1; // sin limite

    // parse args simples: --com COM3 --baud 9600 --max-lines N
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--com") == 0 && i+1 < argc) {
            com = argv[++i];
        } else if (strcmp(argv[i], "--baud") == 0 && i+1 < argc) {
            baud = (unsigned int)atoi(argv[++i]);
        } else if (strcmp(argv[i], "--max-lines") == 0 && i+1 < argc) {
            maxLines = atoi(argv[++i]);
        }
    }

    printf("Conectando a puerto %s a %u baudios...\n", com, baud);

    SerialPort sp;
    if (!sp.open(com, baud)) {
        fprintf(stderr, "No se pudo abrir el puerto.\n");
        return 1;
    }

    printf("Conexión establecida. Esperando tramas...\n\n");

    ListaDeCarga carga;
    RotorDeMapeo rotor;

    char line[128];
    int lineCount = 0;

    while (1) {
        int n = sp.readLine(line, sizeof(line), 5000); // 5s timeout
        if (n < 0) {
            fprintf(stderr, "Error leyendo del puerto.\n");
            break;
        }
        if (n == 0) {
            // timeout sin datos; continuar esperando
            continue;
        }

        // fin de flujo si llega "END" (case-insensitive)
        char upperLine[128];
        strncpy(upperLine, line, sizeof(upperLine)-1);
        upperLine[sizeof(upperLine)-1] = '\0';
        toUpperInPlace(upperLine);
        if (strcmp(upperLine, "END") == 0) {
            printf("---\nFlujo de datos terminado.\n");
            break;
        }

        struct Parsed p = parse_line(line);
        if (!p.valid) {
            printf("Trama mal formada: [%s] -> IGNORADA.\n", line);
            continue;
        }

        TramaBase* trama = NULL;
        char descripcion[64];
        descripcion[0] = '\0';

        if (p.tipo == 'L') {
            trama = new TramaLoad(p.ch);
            snprintf(descripcion, sizeof(descripcion), "L,%c", (p.ch==' ')?' ':p.ch);
        } else {
            trama = new TramaMap(p.num);
            snprintf(descripcion, sizeof(descripcion), "M,%d", p.num);
        }

        // Mensaje inicial
        printf("Trama recibida: [%s] -> Procesando... ", descripcion);

        if (p.tipo == 'M') {
            int oldOff = rotor.getOffsetFromAnchor();
            trama->procesar(&carga, &rotor);
            int newOff = rotor.getOffsetFromAnchor();
            int delta = newOff - oldOff;
            // normalizar delta a [-25,25]
            if (delta > 13) delta -= 26; else if (delta < -13) delta += 26;
            printf("-> ROTANDO ROTOR %s%d. (Ahora 'A' se mapea a '%c')\n\n", (p.num>=0?"+":""), p.num, rotor.getMapeo('A'));
        } else if (p.tipo == 'L') {
            char antes = p.ch;
            // aplicar procesar que inserta
            trama->procesar(&carga, &rotor);
            char despues = rotor.getMapeo(antes);
            // imprimir mensaje parcial con brackets
            printf("-> Fragmento '%c' decodificado como '%c'. Mensaje: ", (antes==' ')?' ':antes, despues);
            carga.imprimirMensajeBrackets();
        }

        delete trama;
        trama = NULL;

        ++lineCount;
        if (maxLines > 0 && lineCount >= maxLines) {
            printf("---\nMaximo de lineas alcanzado.\n");
            break;
        }
    }

    printf("---\nMENSAJE OCULTO ENSAMBLADO:\n");
    carga.imprimirMensaje();
    printf("---\nLiberando memoria... Sistema apagado.\n");

    sp.close();
    return 0;
}
