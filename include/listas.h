#ifndef LISTAS_H
#define LISTAS_H

#include <stddef.h>

/**
 * \brief Nodo para una lista doblemente enlazada de caracteres.
 */
struct NodoChar {
    char valor;
    NodoChar* prev;
    NodoChar* next;
    explicit NodoChar(char v) : valor(v), prev(NULL), next(NULL) {}
};

/**
 * \brief Lista Doblemente Enlazada para almacenar los caracteres decodificados.
 */
class ListaDeCarga {
    NodoChar* head;
    NodoChar* tail;
    size_t     count;
public:
    ListaDeCarga();
    ~ListaDeCarga();

    void insertarAlFinal(char c);
    void imprimirMensaje() const;
    void imprimirMensajeBrackets() const; // imprime [A][B]...

    // Utilidad
    size_t size() const { return count; }
};

/**
 * \brief Nodo del rotor (lista circular doble) con una letra fija A-Z.
 */
struct NodoRotor {
    char letra;           // Letra fija en esta posición física
    NodoRotor* prev;      // Enlace previo
    NodoRotor* next;      // Enlace siguiente
    explicit NodoRotor(char c) : letra(c), prev(NULL), next(NULL) {}
};

/**
 * \brief Rotor de mapeo: lista circular doble A-Z. Mantiene dos punteros:
 * - anchor: apunta SIEMPRE al nodo con 'A' (referencia de entrada)
 * - head: posición 'cero' del mapeo actual (a donde se mapea 'A')
 *
 * Mapeo: para un caracter de entrada con desplazamiento i desde 'A', devuelve
 * la letra en la posición head+i (mod 26). Caracteres fuera de A-Z se devuelven tal cual.
 */
class RotorDeMapeo {
    NodoRotor* anchor;  // nodo físico de 'A' (no rota)
    NodoRotor* head;    // posición cero del mapeo (rota con M,n)
    int size;           // normalmente 26

public:
    RotorDeMapeo();
    ~RotorDeMapeo();

    void rotar(int n);
    char getMapeo(char in) const;

    // debug
    int getOffsetFromAnchor() const; // cuántas posiciones desde anchor hasta head
};

#endif // LISTAS_H
