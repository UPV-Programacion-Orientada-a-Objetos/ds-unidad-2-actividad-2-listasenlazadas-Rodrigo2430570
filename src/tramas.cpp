#include "tramas.h"
#include "listas.h"
#include <stdio.h>

void TramaLoad::procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) {
    if (!carga || !rotor) return;
    char dec = rotor->getMapeo(dato);
    carga->insertarAlFinal(dec);
}

void TramaMap::procesar(ListaDeCarga* /*carga*/, RotorDeMapeo* rotor) {
    if (!rotor) return;
    rotor->rotar(desplazamiento);
}
