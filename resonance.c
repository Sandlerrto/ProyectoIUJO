/*
 * ════════════════════════════════════════════════════════════════
 *   RESONANCE: BLOOD DEBT  —  Simulador Completo v0.5.1
 *   Lenguaje: C 
 * ════════════════════════════════════════════════════════════════
 *
 *  CONCEPTOS APLICADOS:
 *  A. Informática  : tipo de variables, structs, punteros, arreglos, ciclos, condicionales
 *  B. Matemática   : polinomios, logaritmos, valor absoluto, inecuaciones
 *  C. Proposicional: conectivos &&/||, cuantificadores, teoría de conjuntos
 *  D. Dinámica     : catálogo 12, equipos 5, buffs/nerfs, relevo, stats
 *
 *  FACCIONES:
 *   0 VIGILANTES   → PROTOCOLO ESCUDO   (+25% DEF, reduce ventaja a x1.2)
 *   1 DISONANTES   → PULSO CAÓTICO      (+20% ATQ, 30% parálisis)
 *   2 SINDICATO    → PROTOCOLO NULO     (+15% ATQ/HAB, penetra 20% DEF)
 *   3 ARQUITECTOS  → ECO ANCESTRAL      (+20% HP, legado 15% al morir)
 *
 *  CICLO DE VENTAJA (x1.5):
 *   VIGILANTES → DISONANTES → SINDICATO → ARQUITECTOS → VIGILANTES
 * ════════════════════════════════════════════════════════════════
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

/* ── Colores ANSI ── */
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[91m"
#define GREEN   "\033[92m"
#define YELLOW  "\033[93m"
#define BLUE    "\033[94m"
#define MAGENTA "\033[95m"
#define CYAN    "\033[96m"
#define WHITE   "\033[97m"
#define DIM     "\033[2m"

/* ── Constantes de mejora ── */
#define VIG_DEF_BONUS        0.25f
#define VIG_VENTAJA_REDUCIDA 1.20f
#define VIG_VENTAJA_NORMAL   1.50f
#define DIS_ATQ_BONUS        0.20f
#define DIS_PARALISIS_PROB   30
#define SIN_ATQ_BONUS        0.15f
#define SIN_HAB_BONUS        0.15f
#define SIN_PENET_DEF        0.20f
#define ARQ_HP_BONUS         0.20f
#define ARQ_LEGADO_RATIO     0.15f

/* ── Enumeraciones ── */
typedef enum { VIGILANTES=0, DISONANTES=1, SINDICATO=2, ARQUITECTOS=3 } Faccion;
typedef enum { GUERRERO=0, TANQUE=1, MAGO=2 } Rol;

/* ── Estados (bitmask) ── */
#define ESTADO_NORMAL    0x00
#define ESTADO_VENENO    0x01
#define ESTADO_PARALISIS 0x02
#define ESTADO_MIEDO     0x04
#define ESTADO_LENTITUD  0x08

/* ════════════════════════════════════════
 *  ESTRUCTURAS
 * ════════════════════════════════════════ */
typedef struct {
    int     id;
    char    nombre[30];
    Faccion faccion;
    Rol     rol;
    int     hp_max;
    int     hp_actual;
    float   ataque;
    float   defensa;
    float   habilidad;
    int     estado;         /* bitmask de estados alterados */
    int     vivo;
    int     mejora_activa;
    int     dano_total;     /* estadística acumulada */
} Campeon;

typedef struct {
    char    nombre[30];
    Campeon integrantes[5];
    int     size;
    int     turno;          /* índice del combatiente actual */
    int     dano_total;     /* estadística del equipo */
    int     bajas;
} Equipo;

/* ════════════════════════════════════════
 *  CATÁLOGO BASE (12 campeones)
 * ════════════════════════════════════════ */
Campeon catalogo_base[12] = {
 /* id  nombre                faccion      rol       hpM  hpA   atq    def    hab   est  viv  mej  dtot */
 {  1, "Samuel Greenwood",  VIGILANTES,  GUERRERO, 150, 150,  85.0f, 40.0f, 90.0f, 0,   1,   0,   0 },
 {  2, "Valeria Sync",      VIGILANTES,  MAGO,     100, 100,  95.0f, 25.0f, 95.0f, 0,   1,   0,   0 },
 {  3, "Kaelen Ward",       VIGILANTES,  TANQUE,   180, 180,  55.0f, 75.0f, 50.0f, 0,   1,   0,   0 },
 {  4, "Zarek Pulse",       DISONANTES,  GUERRERO, 140, 140,  80.0f, 45.0f, 70.0f, 0,   1,   0,   0 },
 {  5, "Lyra Void",         DISONANTES,  MAGO,     110, 110,  90.0f, 30.0f, 92.0f, 0,   1,   0,   0 },
 {  6, "Vax Shard",         DISONANTES,  TANQUE,   200, 200,  50.0f, 85.0f, 40.0f, 0,   1,   0,   0 },
 {  7, "Yuki Cipher",       SINDICATO,   MAGO,      95,  95, 105.0f, 20.0f, 98.0f, 0,   1,   0,   0 },
 {  8, "Dax Ironframe",     SINDICATO,   TANQUE,   190, 190,  60.0f, 80.0f, 45.0f, 0,   1,   0,   0 },
 {  9, "Sombra Null",       SINDICATO,   GUERRERO, 130, 130, 100.0f, 35.0f, 85.0f, 0,   1,   0,   0 },
 { 10, "Aria Resonant",     ARQUITECTOS, MAGO,     105, 105,  88.0f, 28.0f, 88.0f, 0,   1,   0,   0 },
 { 11, "Molo Dustcore",     ARQUITECTOS, TANQUE,   210, 210,  45.0f, 90.0f, 35.0f, 0,   1,   0,   0 },
 { 12, "Cipher Echo",       ARQUITECTOS, GUERRERO, 135, 135,  78.0f, 50.0f, 65.0f, 0,   1,   0,   0 },
};

/* ════════════════════════════════════════
 *  UTILIDADES DE TEXTO
 * ════════════════════════════════════════ */
const char *nombre_faccion(Faccion f) {
    switch(f) {
        case VIGILANTES:  return "Vigilantes del Código";
        case DISONANTES:  return "Los Disonantes";
        case SINDICATO:   return "Sindicato Binario";
        case ARQUITECTOS: return "Arquitectos de Memoria";
        default:          return "???";
    }
}
const char *color_faccion(Faccion f) {
    switch(f) {
        case VIGILANTES:  return CYAN;
        case DISONANTES:  return RED;
        case SINDICATO:   return WHITE;
        case ARQUITECTOS: return MAGENTA;
        default:          return RESET;
    }
}
const char *nombre_rol(Rol r) {
    switch(r) {
        case GUERRERO: return "Guerrero";
        case TANQUE:   return "Tanque";
        case MAGO:     return "Mago";
        default:       return "???";
    }
}
const char *icono_faccion(Faccion f) {
    switch(f) {
        case VIGILANTES:  return "[VIG]";
        case DISONANTES:  return "[DIS]";
        case SINDICATO:   return "[SIN]";
        case ARQUITECTOS: return "[ARQ]";
        default:          return "[???]";
    }
}

void separador() {
    printf(DIM "  ────────────────────────────────────────────────────\n" RESET);
}
void separador_doble() {
    printf(DIM "  ════════════════════════════════════════════════════\n" RESET);
}

/* Barra de progreso HP */
void barra_hp(int actual, int maximo, const char *color) {
    int total = 20;
    int lleno = (maximo > 0) ? (actual * total / maximo) : 0;
    printf("%s[", color);
    for (int i = 0; i < total; i++)
        printf(i < lleno ? "█" : "░");
    printf("]" RESET " %d/%d", actual, maximo);
}

/* Barra de defensa */
void barra_def(float def, float def_max, const char *color) {
    int total = 10;
    int lleno = (def_max > 0) ? (int)(def * total / def_max) : 0;
    if (lleno > total) lleno = total;
    printf("%s[", color);
    for (int i = 0; i < total; i++)
        printf(i < lleno ? "▪" : "·");
    printf("]" RESET " %.0f", def);
}

/* ════════════════════════════════════════
 *  MOSTRAR CATÁLOGO
 * ════════════════════════════════════════ */
void mostrar_catalogo() {
    printf("\n");
    printf(BOLD CYAN "  ╔══════════════════════════════════════════════════════════════════════╗\n");
    printf(         "  ║          RESONANCE: BLOOD DEBT  —  CATÁLOGO DE CAMPEONES            ║\n");
    printf(         "  ╠══╦══════════════════════╦═══════════════════════╦══════╦══════╦═════╣\n");
    printf(         "  ║ID║ NOMBRE               ║ FACCIÓN               ║  HP  ║  ATQ ║ DEF ║\n");
    printf(         "  ╠══╬══════════════════════╬═══════════════════════╬══════╬══════╬═════╣\n" RESET);

    for (int i = 0; i < 12; i++) {
        Campeon *c = &catalogo_base[i];
        const char *col = color_faccion(c->faccion);
        /* línea separadora entre facciones */
        if (i > 0 && c->faccion != catalogo_base[i-1].faccion)
            printf(DIM "  ╠══╬══════════════════════╬═══════════════════════╬══════╬══════╬═════╣\n" RESET);

        printf("  ║%s%2d%s║ %-21s║ %s%-22s%s║%5d ║%5.0f ║%4.0f ║\n",
               col, c->id, RESET,
               c->nombre,
               col, nombre_faccion(c->faccion), RESET,
               c->hp_max, c->ataque, c->defensa);
    }
    printf(BOLD CYAN "  ╚══╩══════════════════════╩═══════════════════════╩══════╩══════╩═════╝\n\n" RESET);

    /* Leyenda de facciones */
    printf("  %s[VIG]%s Vigilantes  %s[DIS]%s Disonantes  %s[SIN]%s Sindicato  %s[ARQ]%s Arquitectos\n\n",
           CYAN,RESET, RED,RESET, WHITE,RESET, MAGENTA,RESET);
}

/* ════════════════════════════════════════
 *  FORMACIÓN DE EQUIPOS
 * ════════════════════════════════════════ */
int id_ya_elegido(int *elegidos, int n, int id) {
    for (int i = 0; i < n; i++)
        if (elegidos[i] == id) return 1;
    return 0;
}

void formar_equipo(Equipo *e, int num_jugador) {
    int elegidos[5];
    int n = 0;

    printf(BOLD "\n  ╔══════════════════════════════════╗\n");
    printf(     "  ║  JUGADOR %d — FORMA TU EQUIPO    ║\n", num_jugador);
    printf(     "  ╚══════════════════════════════════╝\n\n" RESET);

    printf("  Ingresa el nombre de tu equipo: ");
    scanf(" %29[^\n]", e->nombre);

    printf("\n  Selecciona %s5 campeones%s (ingresa IDs del 1 al 12, sin repetir):\n\n",
           YELLOW, RESET);

    while (n < 5) {
        int id;
        printf("  Campeón %d/5 → ID: ", n + 1);
        if (scanf("%d", &id) != 1) { while(getchar()!='\n'); continue; }

        if (id < 1 || id > 12) {
            printf(RED "  ✗ ID inválido. Ingresa un número entre 1 y 12.\n" RESET);
            continue;
        }
        if (id_ya_elegido(elegidos, n, id)) {
            printf(RED "  ✗ Ya elegiste ese campeón.\n" RESET);
            continue;
        }

        elegidos[n] = id;
        e->integrantes[n] = catalogo_base[id - 1]; /* copia del catálogo */
        const char *col = color_faccion(e->integrantes[n].faccion);
        printf(GREEN "  ✓ %s%s%s (%s) añadido.\n" RESET,
               col, e->integrantes[n].nombre, RESET,
               nombre_faccion(e->integrantes[n].faccion));
        n++;
    }

    e->size       = 5;
    e->turno      = 0;
    e->dano_total = 0;
    e->bajas      = 0;

    /* Mostrar resumen del equipo */
    printf("\n  " BOLD "Equipo \"%s\":\n" RESET, e->nombre);
    for (int i = 0; i < 5; i++) {
        Campeon *c = &e->integrantes[i];
        printf("    %s%s %-22s%s %s | HP:%d ATQ:%.0f DEF:%.0f HAB:%.0f\n",
               color_faccion(c->faccion), icono_faccion(c->faccion),
               c->nombre, RESET,
               nombre_rol(c->rol),
               c->hp_max, c->ataque, c->defensa, c->habilidad);
    }
}

/* ════════════════════════════════════════
 *  MEJORAS DE FACCIÓN
 * ════════════════════════════════════════ */
void aplicar_protocolo_escudo(Campeon *c) {
    c->defensa       *= (1.0f + VIG_DEF_BONUS);
    c->mejora_activa  = 1;
    printf("    %s⬆ PROTOCOLO ESCUDO%s — DEF: %.1f (+25%%)\n", CYAN, RESET, c->defensa);
}
void aplicar_pulso_caotico(Campeon *c) {
    c->ataque        *= (1.0f + DIS_ATQ_BONUS);
    c->mejora_activa  = 1;
    printf("    %s⬆ PULSO CAÓTICO%s   — ATQ: %.1f (+20%%) | Parálisis 30%%\n", RED, RESET, c->ataque);
}
void aplicar_protocolo_nulo(Campeon *c) {
    c->ataque        *= (1.0f + SIN_ATQ_BONUS);
    c->habilidad     *= (1.0f + SIN_HAB_BONUS);
    c->mejora_activa  = 1;
    printf("    %s⬆ PROTOCOLO NULO%s  — ATQ: %.1f (+15%%) HAB: %.1f (+15%%) | Penet 20%%\n",
           WHITE, RESET, c->ataque, c->habilidad);
}
void aplicar_eco_ancestral(Campeon *c) {
    int bonus     = (int)(c->hp_max * ARQ_HP_BONUS);
    c->hp_max    += bonus;
    c->hp_actual += bonus;
    c->mejora_activa = 1;
    printf("    %s⬆ ECO ANCESTRAL%s   — HP: %d (+20%%) | Legado 15%% al morir\n",
           MAGENTA, RESET, c->hp_max);
}
void aplicar_mejora(Campeon *c) {
    switch (c->faccion) {
        case VIGILANTES:  aplicar_protocolo_escudo(c); break;
        case DISONANTES:  aplicar_pulso_caotico(c);    break;
        case SINDICATO:   aplicar_protocolo_nulo(c);   break;
        case ARQUITECTOS: aplicar_eco_ancestral(c);    break;
    }
}

void fase_potenciacion(Equipo *e) {
    printf("\n  %s╔══ POTENCIACIÓN: %s ══╗%s\n", YELLOW, e->nombre, RESET);
    for (int i = 0; i < 5; i++) {
        printf("  %s%-22s%s\n",
               color_faccion(e->integrantes[i].faccion),
               e->integrantes[i].nombre, RESET);
        aplicar_mejora(&e->integrantes[i]);
    }
    printf("  %s╚════════════════════════════╝%s\n", YELLOW, RESET);
}

/* ════════════════════════════════════════
 *  LÓGICA DE COMBATE
 * ════════════════════════════════════════ */

/* C. Conectivos lógicos — tabla de ventaja elemental */
int tiene_ventaja(Faccion atk, Faccion def) {
    return (atk == VIGILANTES  && def == DISONANTES)  ||
           (atk == DISONANTES  && def == SINDICATO)   ||
           (atk == SINDICATO   && def == ARQUITECTOS) ||
           (atk == ARQUITECTOS && def == VIGILANTES);
}

/* C. Cuantificador — ¿existe al menos un combatiente vivo? */
int existe_vivo(Equipo *e) {
    for (int i = 0; i < e->size; i++)
        if (e->integrantes[i].vivo) return 1;
    return 0;
}

/* Siguiente combatiente vivo en el equipo */
Campeon *siguiente_vivo(Equipo *e) {
    for (int i = 0; i < e->size; i++)
        if (e->integrantes[i].vivo) return &e->integrantes[i];
    return NULL;
}

/* Siguiente vivo DESPUÉS del índice dado (para legado ARQ) */
Campeon *siguiente_vivo_tras(Equipo *e, int idx) {
    for (int i = idx + 1; i < e->size; i++)
        if (e->integrantes[i].vivo) return &e->integrantes[i];
    return NULL;
}

int indice_campeon(Equipo *e, Campeon *c) {
    for (int i = 0; i < e->size; i++)
        if (&e->integrantes[i] == c) return i;
    return -1;
}

/* Mostrar estado de un combatiente en pantalla */
void mostrar_combatiente(Campeon *c, const char *lado) {
    const char *col = color_faccion(c->faccion);
    printf("  %s%-6s%s %s%-22s%s %s\n",
           col, icono_faccion(c->faccion), RESET,
           col, c->nombre, RESET, lado);
    printf("         HP  "); barra_hp(c->hp_actual, c->hp_max, col);  printf("\n");
    printf("         DEF "); barra_def(c->defensa, (float)c->hp_max * 0.6f, col); printf("\n");
}

/*
 * CALCULAR DAÑO — integra todas las mecánicas matemáticas:
 *
 *  B1. Poder (polinomial): Poder = 2*(Atq/10)^2 + 3*(Atq/10) + 5
 *  B2. Escala (logarítmica): Daño = Poder * log10(Hab + 1)
 *  B3. Ventaja elemental: x1.5 (o x1.2 si defensor VIG con mejora)
 *  B4. Penetración SIN: DEF_efect = DEF * 0.80
 *  B5. Mitigación (valor absoluto): |DEF - Hab*0.1|
 *  B6. Condición de impacto (inecuación): Daño <= DEF → escudo / else → HP
 */
void calcular_dano(Campeon *atk, Campeon *def, Equipo *equipo_def) {
    /* B1 — Poder base polinomial */
    float a     = atk->ataque / 10.0f;
    float poder = (2.0f * a * a) + (3.0f * a) + 5.0f;

    /* B2 — Escala logarítmica por habilidad */
    float dano  = poder * log10f(atk->habilidad + 1.0f);

    /* B3 — Multiplicador ventaja elemental */
    float mult = 1.0f;
    if (tiene_ventaja(atk->faccion, def->faccion)) {
        if (def->faccion == VIGILANTES && def->mejora_activa)
            mult = VIG_VENTAJA_REDUCIDA;   /* VIG-001 reduce a x1.2 */
        else
            mult = VIG_VENTAJA_NORMAL;
    }
    dano *= mult;

    /* B4 — Penetración de DEF (SIN-003) */
    float def_efect = (atk->faccion == SINDICATO && atk->mejora_activa)
                      ? def->defensa * (1.0f - SIN_PENET_DEF)
                      : def->defensa;

    /* B5 — Mitigación con valor absoluto */
    float mitigacion = fabsf(def_efect - (atk->habilidad * 0.1f));

    /* Log de combate */
    printf("\n  %s%-22s%s ──ATK──▶  %s%-22s%s\n",
           color_faccion(atk->faccion), atk->nombre, RESET,
           color_faccion(def->faccion), def->nombre, RESET);

    if (mult > 1.0f)
        printf("  %s  ⚡ VENTAJA ELEMENTAL ×%.2f%s\n", YELLOW, mult, RESET);
    if (atk->faccion == SINDICATO && atk->mejora_activa)
        printf("  %s  🔩 PENETRACIÓN DE DEFENSA (–20%%)%s\n", WHITE, RESET);

    printf("  %s  Poder:%.1f  Daño:%.1f  DEF_efect:%.1f  Mitig:%.1f%s\n",
           DIM, poder, dano, def_efect, mitigacion, RESET);

    /* B6 — Condición de impacto (inecuación) */
    int impacto_hp = 0;
    if (dano <= mitigacion) {
        def->defensa -= dano;
        if (def->defensa < 0.0f) def->defensa = 0.0f;
        printf("  %s  🛡 Escudo absorbe %.0f daño  | DEF restante: %.1f%s\n",
               CYAN, dano, def->defensa, RESET);
    } else {
        impacto_hp    = (int)(dano - mitigacion);
        def->defensa  = 0.0f;
        int hp_antes  = def->hp_actual;
        def->hp_actual -= impacto_hp;
        if (def->hp_actual < 0) def->hp_actual = 0;
        printf("  %s  💥 Impacto HP: %d → %d  (–%d)%s\n",
               RED, hp_antes, def->hp_actual, impacto_hp, RESET);
    }

    /* Acumular estadísticas */
    atk->dano_total       += impacto_hp;
    equipo_def->dano_total += impacto_hp; /* daño recibido por el equipo */

    /* DIS-002 — Intentar parálisis tras el ataque */
    if (atk->faccion == DISONANTES && atk->mejora_activa && def->vivo) {
        int roll = rand() % 100;
        if (roll < DIS_PARALISIS_PROB) {
            def->estado |= ESTADO_PARALISIS;
            printf("  %s  ⚡ PULSO CAÓTICO — %s sufre PARÁLISIS (roll:%d)%s\n",
                   RED, def->nombre, roll, RESET);
        }
    }

    /* Verificar derrota */
    if (def->hp_actual <= 0) {
        def->vivo = 0;
        equipo_def->bajas++;
        printf("  %s  ☠  %s ha sido derrotado.%s\n", BOLD RED, def->nombre, RESET);

        /* ARQ-004 — Eco Ancestral: legado al siguiente aliado */
        if (def->faccion == ARQUITECTOS && def->mejora_activa) {
            int idx = indice_campeon(equipo_def, def);
            Campeon *sig = siguiente_vivo_tras(equipo_def, idx);
            if (sig == NULL) sig = siguiente_vivo(equipo_def);
            if (sig != NULL && sig != def) {
                int legado = (int)(def->hp_max * ARQ_LEGADO_RATIO);
                sig->hp_actual += legado;
                if (sig->hp_actual > sig->hp_max) sig->hp_actual = sig->hp_max;
                printf("  %s  🧠 ECO ANCESTRAL — %s hereda %d HP → %d/%d%s\n",
                       MAGENTA, sig->nombre, legado,
                       sig->hp_actual, sig->hp_max, RESET);
            }
        }
    }
}

/* Verificar y procesar estado de parálisis */
int procesar_paralisis(Campeon *c) {
    if (c->estado & ESTADO_PARALISIS) {
        c->estado &= ~ESTADO_PARALISIS;
        printf("  %s  ⚡ %s está PARALIZADO — pierde su turno.%s\n",
               YELLOW, c->nombre, RESET);
        return 1;
    }
    return 0;
}

/* ════════════════════════════════════════
 *  MOSTRAR ESTADO ACTUAL DE AMBOS EQUIPOS
 * ════════════════════════════════════════ */
void mostrar_estado_equipos(Equipo *a, Equipo *b) {
    printf("\n");
    separador_doble();
    printf("  %-30s%30s\n",
           a->nombre, b->nombre);
    separador();
    for (int i = 0; i < 5; i++) {
        Campeon *ca = &a->integrantes[i];
        Campeon *cb = &b->integrantes[i];
        const char *ca_col = ca->vivo ? color_faccion(ca->faccion) : DIM;
        const char *cb_col = cb->vivo ? color_faccion(cb->faccion) : DIM;
        printf("  %s%-22s%s%4d/%-5d  %s%-22s%s%4d/%d\n",
               ca_col, ca->nombre, RESET, ca->hp_actual, ca->hp_max,
               cb_col, cb->nombre, RESET, cb->hp_actual, cb->hp_max);
    }
    separador_doble();
    printf("\n");
}

/* ════════════════════════════════════════
 *  BATALLA POR RELEVOS
 * ════════════════════════════════════════ */
void ejecutar_batalla(Equipo *a, Equipo *b) {
    int turno = 1;
    int turno_eq = 0; /* 0 = equipo A ataca, 1 = equipo B ataca */

    printf("\n");
    printf(BOLD RED "  ╔══════════════════════════════════════════════════════╗\n");
    printf(        "  ║          ⚔  BATALLA INICIADA  ⚔                     ║\n");
    printf(        "  ║   %-20s  VS  %-20s║\n", a->nombre, b->nombre);
    printf(        "  ╚══════════════════════════════════════════════════════╝\n\n" RESET);

    /* Bucle principal de batalla — mientras ambos equipos tengan vivos */
    while (existe_vivo(a) && existe_vivo(b)) {

        Campeon *atacante, *defensor;
        Equipo  *eq_def;

        if (turno_eq == 0) {
            atacante = siguiente_vivo(a);
            defensor = siguiente_vivo(b);
            eq_def   = b;
        } else {
            atacante = siguiente_vivo(b);
            defensor = siguiente_vivo(a);
            eq_def   = a;
        }

        /* Si no hay combatientes disponibles, salir */
        if (atacante == NULL || defensor == NULL) break;

        printf(BOLD "\n  ── TURNO %d ──\n" RESET, turno);

        /* Verificar parálisis del atacante */
        if (procesar_paralisis(atacante)) {
            turno_eq = 1 - turno_eq;
            turno++;
            continue;
        }

        /* Mostrar estado actual */
        mostrar_combatiente(atacante, "◀ ATACA");
        mostrar_combatiente(defensor, "▶ DEFIENDE");

        /* Ejecutar ataque */
        calcular_dano(atacante, defensor, eq_def);

        /* Pequeña pausa visual */
        printf("\n");

        /* Si el defensor cayó, mostrar quién lo reemplaza */
        if (!defensor->vivo) {
            Campeon *nuevo = siguiente_vivo(eq_def);
            if (nuevo != NULL) {
                printf("  %s  → %s entra al campo.%s\n\n",
                       color_faccion(nuevo->faccion), nuevo->nombre, RESET);
            }
        }

        /* Mostrar estado de equipos cada 3 turnos */
        if (turno % 3 == 0)
            mostrar_estado_equipos(a, b);

        turno_eq = 1 - turno_eq;
        turno++;

        /* Límite de seguridad */
        if (turno > 200) {
            printf(YELLOW "  [Sistema] Límite de turnos alcanzado.\n" RESET);
            break;
        }
    }

    printf(BOLD "\n  ╔══════════════════════════════════════════╗\n");
    printf(     "  ║         ⚔  BATALLA TERMINADA  ⚔         ║\n");
    printf(     "  ╚══════════════════════════════════════════╝\n\n" RESET);
}

/* ════════════════════════════════════════
 *  ESTADÍSTICAS FINALES
 * ════════════════════════════════════════ */
void estadisticas_finales(Equipo *a, Equipo *b) {
    /* Determinar ganador — C. cuantificador */
    Equipo *ganador = existe_vivo(a) ? a : (existe_vivo(b) ? b : NULL);
    Equipo *perdedor = (ganador == a) ? b : a;

    printf("\n");
    printf(BOLD YELLOW "  ╔══════════════════════════════════════════════════════╗\n");
    printf(            "  ║              ESTADÍSTICAS FINALES                   ║\n");
    printf(            "  ╚══════════════════════════════════════════════════════╝\n\n" RESET);

    if (ganador != NULL) {
        printf(BOLD GREEN "  🏆 EQUIPO GANADOR: %s\n\n" RESET, ganador->nombre);
    } else {
        printf(BOLD YELLOW "  🤝 EMPATE — Ambos equipos cayeron.\n\n" RESET);
    }

    /* Tabla comparativa de equipos */
    printf(BOLD "  %-28s %-28s\n" RESET, a->nombre, b->nombre);
    separador();

    /* Calcular daño total causado por cada equipo */
    int dano_a = 0, dano_b = 0;
    int vivos_a = 0, vivos_b = 0;
    for (int i = 0; i < 5; i++) {
        dano_a  += a->integrantes[i].dano_total;
        dano_b  += b->integrantes[i].dano_total;
        if (a->integrantes[i].vivo) vivos_a++;
        if (b->integrantes[i].vivo) vivos_b++;
    }

    printf("  Daño total causado : %s%-8d%s   Daño total causado : %s%d%s\n",
           GREEN, dano_a, RESET, GREEN, dano_b, RESET);
    printf("  Bajas sufridas     : %s%-8d%s   Bajas sufridas     : %s%d%s\n",
           RED, a->bajas, RESET, RED, b->bajas, RESET);
    printf("  Sobrevivientes     : %s%-8d%s   Sobrevivientes     : %s%d%s\n",
           CYAN, vivos_a, RESET, CYAN, vivos_b, RESET);

    separador();

    /* Ranking de campeones por daño causado */
    printf(BOLD "\n  RANKING — DAÑO CAUSADO POR CAMPEÓN:\n\n" RESET);

    /* Merge de los 10 combatientes y ordenar por daño (bubble sort) */
    Campeon *todos[10];
    for (int i = 0; i < 5; i++) {
        todos[i]   = &a->integrantes[i];
        todos[i+5] = &b->integrantes[i];
    }
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9 - i; j++)
            if (todos[j]->dano_total < todos[j+1]->dano_total) {
                Campeon *tmp = todos[j];
                todos[j]     = todos[j+1];
                todos[j+1]   = tmp;
            }

    for (int i = 0; i < 10; i++) {
        Campeon *c = todos[i];
        const char *col = color_faccion(c->faccion);
        printf("  %2d. %s%-22s%s %s | Daño: %s%5d%s | %s\n",
               i + 1,
               col, c->nombre, RESET,
               icono_faccion(c->faccion),
               YELLOW, c->dano_total, RESET,
               c->vivo ? GREEN "VIVO" RESET : RED "CAÍDO" RESET);
    }

    /* MVP */
    printf(BOLD YELLOW "\n  ⭐ MVP: %s%s%s — %d daño causado\n" RESET,
           color_faccion(todos[0]->faccion),
           todos[0]->nombre,
           RESET,
           todos[0]->dano_total);

    separador_doble();
    printf(DIM "\n  // FRECUENCIA GREENWOOD — EL ECO QUE SALVÓ AL MUNDO\n\n" RESET);
}

/* ════════════════════════════════════════
 *  MENÚ PRINCIPAL
 * ════════════════════════════════════════ */
void limpiar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pantalla_titulo() {
    printf(BOLD CYAN "\n");
    printf("  ██████╗ ███████╗███████╗ ██████╗ ███╗   ██╗ █████╗ ███╗  ██╗ ██████╗███████╗\n");
    printf("  ██╔══██╗██╔════╝██╔════╝██╔═══██╗████╗  ██║██╔══██╗████╗ ██║██╔════╝██╔════╝\n");
    printf("  ██████╔╝█████╗  ███████╗██║   ██║██╔██╗ ██║███████║██╔██╗██║██║     █████╗  \n");
    printf("  ██╔══██╗██╔══╝  ╚════██║██║   ██║██║╚██╗██║██╔══██║██║╚████║██║     ██╔══╝  \n");
    printf("  ██║  ██║███████╗███████║╚██████╔╝██║ ╚████║██║  ██║██║ ╚███║╚██████╗███████╗\n");
    printf("  ╚═╝  ╚═╝╚══════╝╚══════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝  ╚══╝ ╚═════╝╚══════╝\n");
    printf(RED "\n              B L O O D   D E B T\n" RESET);
    printf(DIM "          El Canto de la Disonancia — Dossier Prohibido\n\n" RESET);
}

int main() {
    srand((unsigned int)time(NULL));

    Equipo equipo_a, equipo_b;
    memset(&equipo_a, 0, sizeof(Equipo));
    memset(&equipo_b, 0, sizeof(Equipo));

    int opcion;
    int equipos_formados  = 0;
    int mejoras_aplicadas = 0;
    int batalla_ejecutada = 0;

    pantalla_titulo();

    do {
        printf(BOLD "\n  ╔══════════════════════════╗\n");
        printf(     "  ║      MENÚ PRINCIPAL      ║\n");
        printf(     "  ╠══════════════════════════╣\n");
        printf(     "  ║  1. Ver Catálogo         ║\n");
        printf(     "  ║  2. Formar Equipos       ║\n");
        printf(     "  ║  3. Fase de Potenciación ║\n");
        printf(     "  ║  4. Ejecutar Batalla     ║\n");
        printf(     "  ║  5. Estadísticas Finales ║\n");
        printf(     "  ║  6. Salir                ║\n");
        printf(     "  ╚══════════════════════════╝\n\n" RESET);

        /* Indicadores de estado */
        printf("  Estado: Equipos[%s] Mejoras[%s] Batalla[%s]\n\n",
               equipos_formados  ? GREEN "✓" RESET : RED "✗" RESET,
               mejoras_aplicadas ? GREEN "✓" RESET : RED "✗" RESET,
               batalla_ejecutada ? GREEN "✓" RESET : RED "✗" RESET);

        printf("  Opción: ");
        if (scanf("%d", &opcion) != 1) { limpiar_buffer(); continue; }
        limpiar_buffer();

        switch (opcion) {

            case 1:
                mostrar_catalogo();
                break;

            case 2:
                mostrar_catalogo();
                printf(BOLD CYAN "\n  ══ JUGADOR 1 ══\n" RESET);
                formar_equipo(&equipo_a, 1);
                printf(BOLD RED "\n  ══ JUGADOR 2 ══\n" RESET);
                formar_equipo(&equipo_b, 2);
                equipos_formados  = 1;
                mejoras_aplicadas = 0;
                batalla_ejecutada = 0;
                printf(GREEN "\n  ✓ Equipos formados correctamente.\n" RESET);
                break;

            case 3:
                if (!equipos_formados) {
                    printf(RED "  ✗ Primero forma los equipos (opción 2).\n" RESET);
                    break;
                }
                fase_potenciacion(&equipo_a);
                fase_potenciacion(&equipo_b);
                mejoras_aplicadas = 1;
                printf(GREEN "\n  ✓ Potenciación aplicada.\n" RESET);
                break;

            case 4:
                if (!equipos_formados) {
                    printf(RED "  ✗ Primero forma los equipos (opción 2).\n" RESET);
                    break;
                }
                if (!mejoras_aplicadas) {
                    printf(YELLOW "  ⚠ No aplicaste mejoras. ¿Continuar sin ellas? (1=Sí / 0=No): " RESET);
                    int resp; scanf("%d", &resp); limpiar_buffer();
                    if (!resp) break;
                }
                ejecutar_batalla(&equipo_a, &equipo_b);
                batalla_ejecutada = 1;
                break;

            case 5:
                if (!batalla_ejecutada) {
                    printf(RED "  ✗ Primero ejecuta la batalla (opción 4).\n" RESET);
                    break;
                }
                estadisticas_finales(&equipo_a, &equipo_b);
                break;

            case 6:
                printf(DIM "\n  // Desconectando frecuencia...\n\n" RESET);
                break;

            default:
                printf(RED "  ✗ Opción inválida.\n" RESET);
        }

    } while (opcion != 6);

    return 0;
}