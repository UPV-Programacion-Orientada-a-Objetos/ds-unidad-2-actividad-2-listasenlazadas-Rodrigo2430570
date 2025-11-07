#ifndef TRAMAS_H
#define TRAMAS_H

#include "listas.h"

/**
 * \brief Clase base abstracta para cualquier trama recibida por el puerto serial.
 */
class TramaBase {
public:
    /**
     * \brief Procesa la trama modificando las estructuras del decodificador.
     */
    virtual void procesar(class ListaDeCarga* carga, class RotorDeMapeo* rotor) = 0;

    /** Destructor virtual obligatorio para limpieza polimórfica */
    virtual ~TramaBase() {}
};

/**
 * \brief Trama de carga: almacena un caracter que debe decodificarse via el rotor.
 */
class TramaLoad : public TramaBase {
    char dato;
public:
    explicit TramaLoad(char c) : dato(c) {}
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override;
};

/**
 * \brief Trama de mapeo: rota el rotor N posiciones (positivas o negativas).
 */
class TramaMap : public TramaBase {
    int desplazamiento;
public:
    explicit TramaMap(int n) : desplazamiento(n) {}
    void procesar(ListaDeCarga* carga, RotorDeMapeo* rotor) override;
};

#endif // TRAMAS_H
