# ProyectoIUJO

# README — RESONANCE: BLOOD DEBT v0.9.0

# Simulador de Combate por Turnos en C

## Autores

Andriu Andueza
Gabriel Gonzalez
Deibyslin Perez
Sebastian Castellano
Kimberly Archila

-----

## Indentacion

- Se usan **4 espacios** por nivel de indentacion.
- Las llaves de apertura `{` van en la linea siguiente al bloque
  (estilo Allman), no al final de la misma linea.

Ejemplo:

```c
void mi_funcion(int x)
{
    if (x > 0)
    {
        printf("positivo");
    }
}
```

-----

## Convenciones de Nomenclatura

### Variables y funciones

- Se usa **snake_case** (palabras separadas por guion bajo).
- Nombres en **ingles** para garantizar compatibilidad y seguir
  estandares internacionales.
- Los nombres deben ser descriptivos e indicar su proposito.

|Tipo      |Convencion     |Ejemplo                    |
|----------|---------------|---------------------------|
|Variables |snake_case     |`hp_current`, `is_alive`   |
|Funciones |snake_case     |`execute_attack()`         |
|Structs   |PascalCase     |`Champion`, `Team`, `Skill`|
|Constantes|UPPER_SNAKE    |`BATTLE_DELAY_SHORT`       |
|Enums     |UPPER_CASE     |`WARDENS`, `DISSONANTS`    |
|Punteros  |prefijo ninguno|`Champion *c`              |

### Structs y tipos

- Los `typedef struct` usan **PascalCase**.
- Los `typedef enum` usan **PascalCase** para el tipo y
  **UPPER_CASE** para sus valores.

### Prefijos de variables globales

- `g_` para variables globales de control (ejemplo: `g_modern`).
- `skl_` para arreglos de habilidades por campeon
  (ejemplo: `skl_samuel`, `skl_lyra`).
- `cards_` para arreglos de cartas por faccion
  (ejemplo: `cards_wardens`, `cards_syndicate`).
- `global_` para instancias globales unicas
  (ejemplo: `global_basic_attack`).

-----

## Estructura del Archivo

El archivo sigue este orden obligatorio:

```
1. Includes y defines (#include, #define, #ifdef)
2. Enumeraciones (typedef enum)
3. Estructuras de datos (typedef struct)
4. Datos globales (catalogos, habilidades, cartas)
5. Prototipos de funciones
6. int main(void)         <- al principio segun requisito del Profesor
7. Implementacion de todas las funciones
```

-----

## Principios de Diseno Aplicados

### DRY (Don’t Repeat Yourself)

El calculo de defensa estaba repetido en multiples lugares asi que hice un cambio.
Se extrajo a `calculate_defense()` para reutilizarlo.

### KISS (Keep It Simple, Stupid)

Cada funcion hace una sola tarea:

- `verify_advantage()` solo verifica ventaja de faccion.
- `calculate_defense()` solo calcula la mitigacion.
- `display_both_fighters()` solo muestra los dos luchadores.

### Nombres significativos

Se evitan nombres como `x`, `temp`, `data`.
Se usan nombres que explican la intencion:

- `hp_impact` en vez de `d`
- `effective_def` en vez de `ed`
- `has_alive_members()` en vez de `check()`

-----

## Compilacion

### Windows (Dev-C++ / MinGW):

```
gcc resonance_blood_debt_v09.c -o resonance.exe
```

### Linux / Mac:

```
gcc resonance_blood_debt_v09.c -o resonance
./resonance
```

-----

## Conceptos Academicos Cubiertos

|Tema                 |Donde aparece en el codigo             |
|---------------------|---------------------------------------|
|Calculo Proposicional|`verify_advantage()`, estados alterados|
|Inferencia Logica    |Ciclo de ventaja, flujo de turnos      |
|Cuantificadores      |`has_alive_members()`, bucles for      |
|Teoria de Conjuntos  |4 facciones como conjuntos disjuntos   |
|Teoria de Grafos     |Ciclo de ventaja como grafo dirigido   |
|Polinomios           |`base_power = 2a^2 + 3a + 5`           |
|Inecuaciones         |`mitigation >= 0` siempre              |
|Valor absoluto minimo|Cota inferior en `calculate_defense()` |





## desing by Sandler