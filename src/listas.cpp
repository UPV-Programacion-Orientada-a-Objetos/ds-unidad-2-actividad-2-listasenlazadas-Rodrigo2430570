#include "listas.h"
#include <stdio.h>
#include <stdlib.h>

ListaDeCarga::ListaDeCarga() : head(NULL), tail(NULL), count(0) {}

ListaDeCarga::~ListaDeCarga() {
    NodoChar* cur = head;
    while (cur) {
        NodoChar* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    head = tail = NULL;
    count = 0;
}

void ListaDeCarga::insertarAlFinal(char c) {
    NodoChar* n = new NodoChar(c);
    if (!tail) {
        head = tail = n;
    } else {
        tail->next = n;
        n->prev = tail;
        tail = n;
    }
    count++;
}

void ListaDeCarga::imprimirMensaje() const {
    const NodoChar* cur = head;
    while (cur) {
        putchar(cur->valor);
        cur = cur->next;
    }
    putchar('\n');
}

void ListaDeCarga::imprimirMensajeBrackets() const {
    const NodoChar* cur = head;
    while (cur) {
        putchar('[');
        putchar(cur->valor);
        putchar(']');
        cur = cur->next;
    }
    putchar('\n');
}

// ---------------- Rotor -----------------

static NodoRotor* crearAnilloAZ(int* outSize, NodoRotor** outAnchorA) {
    const int N = 26;
    NodoRotor* first = NULL;
    NodoRotor* prev = NULL;

    for (int i = 0; i < N; ++i) {
        char letra = 'A' + i;
        NodoRotor* n = new NodoRotor(letra);
        if (!first) first = n;
        if (prev) {
            prev->next = n;
            n->prev = prev;
        }
        prev = n;
    }
    // cerrar anillo
    first->prev = prev;
    prev->next = first;

    if (outSize) *outSize = N;

    // localizar 'A' (es first)
    if (outAnchorA) *outAnchorA = first;

    return first; // también es 'A'
}

RotorDeMapeo::RotorDeMapeo() : anchor(NULL), head(NULL), size(0) {
    anchor = crearAnilloAZ(&size, &anchor);
    head = anchor; // A mapea a A al inicio
}

RotorDeMapeo::~RotorDeMapeo() {
    // liberar anillo de 26 nodos
    if (!anchor) return;
    NodoRotor* cur = anchor->next;
    for (int i = 1; i < size; ++i) {
        NodoRotor* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
    delete anchor;
    anchor = head = NULL;
    size = 0;
}

void RotorDeMapeo::rotar(int n) {
    if (!head || size <= 0) return;

    // normalizar n al rango [0,size)
    int k = n % size;
    if (k < 0) k += size;

    while (k--) {
        head = head->next;
    }
}

int RotorDeMapeo::getOffsetFromAnchor() const {
    // retorna offset tal que avanzar anchor offset veces llega a head
    if (!anchor || !head) return 0;
    int offset = 0;
    const NodoRotor* cur = anchor;
    while (cur != head) {
        cur = cur->next;
        ++offset;
        if (offset > size) break; // seguridad
    }
    return offset % (size == 0 ? 1 : size);
}

char RotorDeMapeo::getMapeo(char in) const {
    if (in >= 'A' && in <= 'Z') {
        // encontrar desplazamiento i desde 'A' (anchor)
        int i = in - 'A';
        const NodoRotor* cur = head;
        // avanzar i posiciones desde head
        while (i--) cur = cur->next;
        return cur->letra;
    }
    // mantener otros caracteres (espacio, signos) sin cambio
    return in;
}
