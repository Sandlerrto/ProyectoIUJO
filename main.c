/*
 * ════════════════════════════════════════════════════════════════
 *   RESONANCE: BLOOD DEBT  —  Simulador Completo v0.8.1
 *   Lenguaje: C 
 * ════════════════════════════════════════════════════════════════
 *
 *  CONCEPTOS APLICADOS:
 *  A. Informática  : tipo de variables, structs, punteros, arreglos, ciclos, condicionales
 *  B. Matemática   : polinomios, logaritmos, valor absoluto, inecuaciones
 *  C. Proposicional: conectivos &&/||, cuantificadores, teoría de conjuntos
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

#ifdef _WIN32
  #define CLEAR_CMD "cls"
  #include <windows.h>
  static int is_modern_terminal(void) 
  {
      if (getenv("WT_SESSION") != NULL) return 1;
      if (getenv("TERM_PROGRAM") != NULL) return 1;
      if (getenv("WSLENV") != NULL) return 1;
      return 0;
  }
  static int g_modern = 0;
  static void enable_ansi(void)
  {
      HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE); DWORD m = 0;
      if(GetConsoleMode(h, &m)) SetConsoleMode(h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
      g_modern = is_modern_terminal();
      if(g_modern) 
	  {
          SetConsoleOutputCP(65001);
          SetConsoleCP(65001);
      }
  }
#else
  #define CLEAR_CMD "clear"
  static int g_modern = 1;
  static void enable_ansi(void)
  {}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define RST  "\033[0m"
#define BOLD "\033[1m"
#define DIM  "\033[2m"
#define RED  "\033[91m"
#define GRN  "\033[92m"
#define YLW  "\033[93m"
#define MAG  "\033[95m"
#define CYN  "\033[96m"
#define WHT  "\033[97m"

typedef enum { WARDENS = 0, DISSONANTS = 1, SYNDICATE = 2, ARCHITECTS = 3 } Faction;
typedef enum { WARRIOR = 0, TANK = 1, MAGE = 2 }                           Role;
typedef enum { MODE_PVP = 0, MODE_PVE = 1 }                                GameMode;

/* Control de Lenguaje: 0 = Spanish, 1 = English */
int current_language = 0;

/* --------------------------------------
 *  DATA STRUCTURES // ESTRUCTURA DE DATOS
 * -------------------------------------- */

typedef struct
{
    char  name_es[30];
    char  name_en[30];
    char  desc_es[100];
    char  desc_en[100];
    float damage_multiplier;
    float buff_percentage;
    int   buff_stat;          /* 0=Atk, 1=Def, 2=Skl, 3=None */
    float nerf_percentage;
    int   nerf_stat;          /* 0=Atk, 1=Def, 2=Skl, 3=None */
    int   apply_poison;
    int   apply_paralysis;
    int   apply_fear;
    int   apply_slow;
    int   status_probability;
    int   healing_percentage;
    int   energy_cost;
    int   is_permanent;
} Skill;

typedef struct 
{
    char  name_es[30];
    char  name_en[30];
    char  desc_es[60];
    char  desc_en[60];
    float mod_attack;
    float mod_defense;
    float mod_skill;
    float mod_hp;
} BuffCard;

typedef struct 
{
    int      id;
    char     name_es[30];
    char     name_en[30];
    Faction  faction;
    Role     role;
    int      hp_max;
    int      hp_current;
    int      energy_max;
    int      energy_current;
    float    attack;
    float    defense;
    float    skill;
    int      poisoned;
    int      paralyzed;
    int      feared;
    int      slowed;
    int      is_alive;
    int      buff_active;
    int      total_damage_dealt;
    Skill    skills[3];
} Champion;

typedef struct 
{
    char     name[30];
    Champion members[5];
    int      count;
    int      total_team_damage;
    int      casualties;
} Team;

/* ------------------------------------------------------------
 *  SKILL DEFINITIONS (3 per champion)
 * ------------------------------------------------------------ */

Skill skl_samuel[3] = 
{
 {"Pulso Sismico","Seismic Pulse","+15%ATK este turno,-10%DEF. x1.3 DMG.","+15%ATK this turn,-10%DEF. x1.3 DMG.",1.3f,0.15f,0,0.10f,1,0,0,0,0,0,0,2,0},
 {"Barrera Codigo","Code Barrier","Sin DMG. +25%DEF,-10%ATK permanente.","No DMG. +25%DEF,-10%ATK permanent.",0.0f,0.25f,1,0.10f,0,0,0,0,0,0,0,2,1},
 {"Resonancia Total","Total Resonance","x1.8 DMG,-20%SKL este turno.","x1.8 DMG,-20%SKL this turn.",1.8f,0.00f,3,0.20f,2,0,0,0,0,0,0,4,0}
};
Skill skl_valeria[3] = 
{
 {"Eco Caotico","Chaotic Echo","x1.4 DMG, 20% paralisis.","x1.4 DMG, 20% paralysis.",1.4f,0.00f,3,0.00f,3,0,1,0,0,20,0,2,0},
 {"Pulso Curativo","Healing Pulse","Sin DMG. Sana 15%HP,-15%ATK permanente.","No DMG. Heals 15%HP,-15%ATK permanent.",0.0f,0.00f,3,0.15f,0,0,0,0,0,0,15,2,1},
 {"Sobrecarga","Overload","x2.0 DMG,-25%SKL este turno.","x2.0 DMG,-25%SKL this turn.",2.0f,0.00f,3,0.25f,2,0,0,0,0,0,0,5,0}
};
Skill skl_kaelen[3] = 
{
 {"Fortaleza","Fortress","Sin DMG. +30%DEF,-20%ATK permanente.","No DMG. +30%DEF,-20%ATK permanent.",0.0f,0.30f,1,0.20f,0,0,0,0,0,0,0,1,1},
 {"Golpe Escudo","Shield Bash","x1.2 DMG, 30% miedo.","x1.2 DMG, 30% fear.",1.2f,0.00f,3,0.00f,3,0,0,1,0,30,0,2,0},
 {"Bastion","Bastion","x1.5 DMG,+20%DEF,-15%SKL permanente.","x1.5 DMG,+20%DEF,-15%SKL permanent.",1.5f,0.20f,1,0.15f,2,0,0,0,0,0,0,3,1}
};
Skill skl_zarek[3] = 
{
 {"Rafaga Caotica","Chaos Burst","x1.3 DMG, 25% paralisis.","x1.3 DMG, 25% paralysis.",1.3f,0.00f,3,0.00f,3,0,1,0,0,25,0,2,0},
 {"Sobrecorriente","Overcurrent","x1.6 DMG,-15%DEF este turno.","x1.6 DMG,-15%DEF this turn.",1.6f,0.00f,3,0.15f,1,0,0,0,0,0,0,3,0},
 {"Descarga Total","Total Discharge","x2.2 DMG, retroceso -10%HP propio.","x2.2 DMG, recoil -10% own HP.",2.2f,0.00f,3,0.00f,3,0,0,0,0,0,-10,4,0}
};
Skill skl_lyra[3] = 
{
 {"Ilusion Sonica","Sonic Illusion","x1.1 DMG, 40% paralisis.","x1.1 DMG, 40% paralysis.",1.1f,0.00f,3,0.00f,3,0,1,0,0,40,0,2,0},
 {"Onda Pura","Pure Wave","x1.5 DMG,+15%SKL,-10%DEF este turno.","x1.5 DMG,+15%SKL,-10%DEF this turn.",1.5f,0.15f,2,0.10f,1,0,0,0,0,0,0,3,0},
 {"Caos Resonante","Chaos Resonance","x1.9 DMG, VENENO 100%,-20%ATK este turno.","x1.9 DMG, POISON 100%,-20%ATK this turn.",1.9f,0.00f,3,0.20f,0,1,0,0,0,100,0,4,0}
};
Skill skl_vax[3] = 
{
 {"Absorber","Absorb","Sin DMG. Regenera 20%HP,-10%ATK permanente.","No DMG. Regen 20%HP,-10%ATK permanent.",0.0f,0.00f,3,0.10f,0,0,0,0,0,0,20,1,1},
 {"Contraimpacto","Counter Impact","x1.2 DMG,+15%DEF,-10%SKL este turno.","x1.2 DMG,+15%DEF,-10%SKL this turn.",1.2f,0.15f,1,0.10f,2,0,0,0,0,0,0,2,0},
 {"Muro Absoluto","Absolute Wall","x0.8 DMG,+40%DEF,-20%ATK permanente.","x0.8 DMG,+40%DEF,-20%ATK permanent.",0.8f,0.40f,1,0.20f,0,0,0,0,0,0,0,3,1}
};
Skill skl_yuki[3] = 
{
 {"Codigo Nulo","Null Code","x1.5 DMG, penetra 30%DEF,-15%SKL este turno.","x1.5 DMG, pen 30%DEF,-15%SKL this turn.",1.5f,0.00f,3,0.15f,2,0,0,0,0,0,0,2,0},
 {"Cifrado","Cipher Strike","x1.8 DMG,-20%ATK este turno.","x1.8 DMG,-20%ATK this turn.",1.8f,0.00f,3,0.20f,0,0,0,0,0,0,0,3,0},
 {"Omega Protocol","Omega Protocol","x2.5 DMG,-30%ATK y DEF permanente.","x2.5 DMG,-30%ATK and DEF permanent.",2.5f,0.00f,3,0.30f,0,0,0,0,0,0,0,5,1}
};
Skill skl_dax[3] = 
{
 {"Blindaje","Armor Up","Sin DMG.+35%DEF,-15%ATK permanente.","No DMG.+35%DEF,-15%ATK permanent.",0.0f,0.35f,1,0.15f,0,0,0,0,0,0,0,1,1},
 {"Impacto Pesado","Heavy Strike","x1.4 DMG, 25% lentitud,-10%SKL este turno.","x1.4 DMG, 25% slow,-10%SKL this turn.",1.4f,0.00f,3,0.10f,2,0,0,0,1,25,0,2,0},
 {"Demolicion","Demolition","x1.7 DMG,-10%DEF enemigo permanente.","x1.7 DMG,-10% enemy DEF permanent.",1.7f,0.00f,3,0.10f,1,0,0,0,0,0,0,3,0}
};
Skill skl_sombra[3] = 
{
 {"Ataque Furtivo","Stealth Strike","x1.6 DMG, penetra 25%DEF,-10%DEF propio.","x1.6 DMG, pen 25%DEF,-10% own DEF.",1.6f,0.00f,3,0.10f,1,0,0,0,0,0,0,2,0},
 {"Veneno Datos","Data Poison","x1.1 DMG, VENENO 100%,-10%SKL este turno.","x1.1 DMG, POISON 100%,-10%SKL this turn.",1.1f,0.00f,3,0.10f,2,1,0,0,0,100,0,2,0},
 {"Ejecucion","Execution","x2.0 DMG si enemigo <30%HP, sino x1.0.","x2.0 DMG if enemy <30%HP, else x1.0.",2.0f,0.00f,3,0.00f,3,0,0,0,0,0,0,4,0}
};
Skill skl_aria[3] = 
{
 {"Eco Sanador","Healing Echo","Sin DMG. Sana 20%HP,-15%ATK permanente.","No DMG. Heals 20%HP,-15%ATK permanent.",0.0f,0.00f,3,0.15f,0,0,0,0,0,0,20,1,1},
 {"Pulso Memoria","Memory Pulse","x1.4 DMG, 20% miedo,-10%DEF este turno.","x1.4 DMG, 20% fear,-10%DEF this turn.",1.4f,0.00f,3,0.10f,1,0,0,1,0,20,0,2,0},
 {"Fragmento Eco","Echo Fragment","x1.9 DMG,+20%SKL,-20%DEF este turno.","x1.9 DMG,+20%SKL,-20%DEF this turn.",1.9f,0.20f,2,0.20f,1,0,0,0,0,0,0,4,0}
};
Skill skl_molo[3] = 
{
 {"Trinchera","Trench","Sin DMG.+40%DEF,-25%ATK permanente.","No DMG.+40%DEF,-25%ATK permanent.",0.0f,0.40f,1,0.25f,0,0,0,0,0,0,0,1,1},
 {"Golpe Tierra","Earth Strike","x1.3 DMG, 30% lentitud,-10%DEF este turno.","x1.3 DMG, 30% slow,-10%DEF this turn.",1.3f,0.00f,3,0.10f,1,0,0,0,1,30,0,2,0},
 {"Seismo","Seism","x1.6 DMG,-15%SKL permanente.","x1.6 DMG,-15%SKL permanent.",1.6f,0.00f,3,0.15f,2,0,0,0,0,0,0,3,1}
};
Skill skl_cipher[3] = 
{
 {"Memoria Antigua","Ancient Memory","x1.2 DMG, sana 10%HP,-10%ATK este turno.","x1.2 DMG, heals 10%HP,-10%ATK this turn.",1.2f,0.00f,3,0.10f,0,0,0,0,0,0,10,2,0},
 {"Codigo Ancestral","Ancestral Code","x1.5 DMG, 25% veneno,-10%SKL este turno.","x1.5 DMG, 25% poison,-10%SKL this turn.",1.5f,0.00f,3,0.10f,2,1,0,0,0,25,0,3,0},
 {"Resonancia Final","Final Resonance","x2.1 DMG, sacrificio -15%HP propio.","x2.1 DMG, sacrifice -15% own HP.",2.1f,0.00f,3,0.00f,3,0,0,0,0,0,-15,5,0}
};

Skill *skills_table[12] = 
{
    skl_samuel, skl_valeria, skl_kaelen,
    skl_zarek,  skl_lyra,    skl_vax,
    skl_yuki,   skl_dax,     skl_sombra,
    skl_aria,   skl_molo,    skl_cipher
};

/* --------------------------------------------
 *  BUFF CARDS DEFINITIONS (4 per faction)
 * -------------------------------------------- */
BuffCard cards_wardens[4] = 
{
 {"Protocolo Escudo I","Shield Protocol I","+20%DEF,-10%ATK","+20%DEF,-10%ATK",0.90f,1.20f,1.00f,1.00f},
 {"Protocolo Escudo II","Shield Protocol II","+30%DEF,-20%ATK","+30%DEF,-20%ATK",0.80f,1.30f,1.00f,1.00f},
 {"Sincronizacion","Synchronize","+15%SKL,-10%DEF","+15%SKL,-10%DEF",1.00f,0.90f,1.15f,1.00f},
 {"Pulso Vital","Vital Pulse","+15%HP,-10%ATK","+15%HP,-10%ATK",0.90f,1.00f,1.00f,1.15f}
};
BuffCard cards_dissonants[4] = 
{
 {"Pulso Caotico I","Chaos Pulse I","+20%ATK,-15%DEF","+20%ATK,-15%DEF",1.20f,0.85f,1.00f,1.00f},
 {"Pulso Caotico II","Chaos Pulse II","+30%ATK,-20%SKL","+30%ATK,-20%SKL",1.30f,1.00f,0.80f,1.00f},
 {"Descarga","Discharge","+20%SKL,-15%ATK","+20%SKL,-15%ATK",0.85f,1.00f,1.20f,1.00f},
 {"Furia Caotica","Chaos Fury","+25%ATK,-15%HP","+25%ATK,-15%HP",1.25f,1.00f,1.00f,0.85f}
};
BuffCard cards_syndicate[4] = 
{
 {"Protocolo Nulo I","Null Protocol I","+15%ATK,+15%SKL,-20%DEF","+15%ATK,+15%SKL,-20%DEF",1.15f,0.80f,1.15f,1.00f},
 {"Protocolo Nulo II","Null Protocol II","+25%ATK,-20%SKL","+25%ATK,-20%SKL",1.25f,1.00f,0.80f,1.00f},
 {"Logica Fria","Cold Logic","+20%SKL,-15%HP","+20%SKL,-15%HP",1.00f,1.00f,1.20f,0.85f},
 {"Binario","Binary","+20%DEF,-20%ATK","+20%DEF,-20%ATK",0.80f,1.20f,1.00f,1.00f}
};
BuffCard cards_architects[4] = 
{
 {"Eco Ancestral I","Ancestral Echo I","+20%HP,-10%ATK","+20%HP,-10%ATK",0.90f,1.00f,1.00f,1.20f},
 {"Eco Ancestral II","Ancestral Echo II","+30%HP,-20%SKL","+30%HP,-20%SKL",1.00f,1.00f,0.80f,1.30f},
 {"Memoria Viva","Living Memory","+20%SKL,-10%HP","+20%SKL,-10%HP",1.00f,1.00f,1.20f,0.90f},
 {"Armadura Eco","Echo Armor","+20%DEF,-10%SKL","+20%DEF,-10%SKL",1.00f,1.20f,0.90f,1.00f}
};

BuffCard *cards_table[4] = 
{
    cards_wardens, cards_dissonants, cards_syndicate, cards_architects
};

/* --------------------------------------
 *  GENERAL CHAMPION CATALOGUE
 * -------------------------------------- */
Champion champions_catalogue[12] = 
{
{1,"Samuel Greenwood","Samuel Greenwood",WARDENS,  WARRIOR,150,150,6,6, 85.0f,40.0f,90.0f,0,0,0,0,1,0,0,{{0}}},
{2,"Valeria Sync",    "Valeria Sync",    WARDENS,  MAGE,   100,100,7,7, 95.0f,25.0f,95.0f,0,0,0,0,1,0,0,{{0}}},
{3,"Kaelen Ward",     "Kaelen Ward",     WARDENS,  TANK,   180,180,5,5, 55.0f,75.0f,50.0f,0,0,0,0,1,0,0,{{0}}},
{4,"Zarek Pulse",     "Zarek Pulse",     DISSONANTS,WARRIOR,140,140,6,6,80.0f,45.0f,70.0f,0,0,0,0,1,0,0,{{0}}},
{5,"Lyra Void",       "Lyra Void",       DISSONANTS,MAGE,  110,110,7,7, 90.0f,30.0f,92.0f,0,0,0,0,1,0,0,{{0}}},
{6,"Vax Shard",       "Vax Shard",       DISSONANTS,TANK,  200,200,5,5, 50.0f,85.0f,40.0f,0,0,0,0,1,0,0,{{0}}},
{7,"Yuki Cipher",     "Yuki Cipher",     SYNDICATE, MAGE,   95, 95,8,8,105.0f,20.0f,98.0f,0,0,0,0,1,0,0,{{0}}},
{8,"Dax Ironframe",   "Dax Ironframe",   SYNDICATE, TANK,  190,190,5,5, 60.0f,80.0f,45.0f,0,0,0,0,1,0,0,{{0}}},
{9,"Sombra Null",     "Shadow Null",     SYNDICATE, WARRIOR,130,130,6,6,100.0f,35.0f,85.0f,0,0,0,0,1,0,0,{{0}}},
{10,"Aria Resonant",  "Aria Resonant",   ARCHITECTS,MAGE,  105,105,7,7, 88.0f,28.0f,88.0f,0,0,0,0,1,0,0,{{0}}},
{11,"Molo Dustcore",  "Molo Dustcore",   ARCHITECTS,TANK,  210,210,5,5, 45.0f,90.0f,35.0f,0,0,0,0,1,0,0,{{0}}},
{12,"Cipher Echo",    "Cipher Echo",     ARCHITECTS,WARRIOR,135,135,6,6,78.0f,50.0f,65.0f,0,0,0,0,1,0,0,{{0}}}
};

/* --------------------------------------
 *  INTERFACE AUXILIARY FUNCTIONS
 * -------------------------------------- */
const char *get_name(Champion *c)
{
    return (current_language == 0) ? c->name_es : c->name_en;
}
const char *get_faction_name(Faction f)
{
    if(current_language == 0)
	{
        if(f==WARDENS) return "Vigilantes del Codigo";
        if(f==DISSONANTS) return "Los Disonantes";
        if(f==SYNDICATE) return "Sindicato Binario";
        return "Arquitectos de Memoria";
    } else {
        if(f==WARDENS) return "Code Wardens";
        if(f==DISSONANTS) return "The Dissonants";
        if(f==SYNDICATE) return "Binary Syndicate";
        return "Memory Architects";
    }
}
const char *get_faction_color(Faction f)
{
    if(f==WARDENS) return CYN;
    if(f==DISSONANTS) return RED;
    if(f==SYNDICATE) return WHT;
    return MAG;
}
const char *get_faction_icon(Faction f)
{
    if(f==WARDENS) return "[WRD]";
    if(f==DISSONANTS) return "[DIS]";
    if(f==SYNDICATE) return "[SYN]";
    return "[ARC]";
}
const char *get_role_name(Role r)
{
    if(current_language == 0)
	{
        if(r==WARRIOR) return "Guerrero";
        if(r==TANK) return "Tanque";
        return "Mago";
    } else {
        if(r==WARRIOR) return "Warrior";
        if(r==TANK) return "Tank";
        return "Mage";
    }
}

void print_divider(void)        { printf(DIM "  ------------------------------------------------\n" RST); }
void print_double_divider(void) { printf(DIM "  ================================================\n" RST); }

void press_to_continue(void) 
{
    if(current_language == 0) printf("\n  ENTER para continuar...");
    else printf("\n  Press ENTER to continue...");
    getchar();
}
void clear_input_buffer(void) 
{
    int c; while((c = getchar()) != '\n' && c != EOF);
}

/* -------------------------------------------------------
 *  BARRAS HP / ENERGIA
 * ------------------------------------------------------- */
void print_hp_bar(int current, int max, const char *color) 
{
    int total_chars = 16;
    int filled = (max > 0) ? (current * total_chars / max) : 0;
    if(filled > total_chars) filled = total_chars;
    printf("%s[", color);
    for(int i = 0; i < total_chars; i++)
	{
        if(i < filled) printf(g_modern ? "\xe2\x96\x88" : "#");
        else           printf(g_modern ? "\xe2\x96\x91" : ".");
    }
    printf("]" RST " %3d/%d", current, max);
}
void print_energy_bar(int current, int max, const char *color) 
{
    int total_chars = 8;
    int filled = (max > 0) ? (current * total_chars / max) : 0;
    if(filled > total_chars) filled = total_chars;
    printf("%s[", color);
    for(int i = 0; i < total_chars; i++)
	{
        if(i < filled) printf("*");
        else           printf(".");
    }
    printf("]" RST " %d/%d", current, max);
}

/* -------------------------------------------------------
 *  PANTALLA DE TITULO
 * ------------------------------------------------------- */
void display_title_screen(void) 
{
    system(CLEAR_CMD);

    if(g_modern) 
	{
        printf(BOLD CYN
        /* R */
        "\n"
        "  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\n"
        "  \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88       \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\n"
        "  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88       \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88 \xe2\x96\x88 \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\n"
        "  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x91\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x91\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88       \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x91\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88\xe2\x96\x91\xe2\x96\x88\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88    \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\n"
        "  \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88   \xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x91\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\n"
        RST);
    } else 
	{
        printf(BOLD CYN
        "\n"
        "  +------+ +------+ +------+ +-----+  +--+  +-+  ++---++ +--+   +-+  +-----+  +------+\n"
        "  |  +-+ | |  +---+ |  +---+ |  +--++ +++++ +++  | +-+ | +++++  +++  + +---+  |  +---+\n"
        "  |  +-+-+ |  +-+   |  +-+   |  |   | |  ++ +++  | | | | ||  ++ +++  |       ||  +-+  \n"
        "  |  +-+ | |  +-+   +----+   |  |   | |  |++++|  | | | | ||  |++++|  |       ||  +-+  \n"
        "  |  | | | |  +---- +  +-+   |  +--+| |  | ++    | +-+ | |   |  ++   | +---+  || +----+\n"
        "  +--+ +-+ +------+ +------+ +-----+  +--+  +--+ +-----+ +--+   +--+ +------ ++------+\n"
        RST);
    }

    printf(RED BOLD "\n              B L O O D   D E B T\n" RST);
    if(current_language == 0) printf(DIM "          El Canto de la Disonancia\n\n" RST);
    else                      printf(DIM "          The Song of Dissonance\n\n" RST);
}

/* --------------------------------------
 *  MODULE: CATALOGUE AND DATA PRINTING
 * -------------------------------------- */
void display_catalogue(void) 
{
    if(current_language == 0)
	{
        printf("\n" BOLD CYN
        "  +--------------------------------------------------------------------------+\n"
        "  |ID| NOMBRE               | FACCION               |  HP | ATK| DEF| SKL| EN|\n"
        "  |--+----------------------+-----------------------+-----+----+----+----+---|\n" RST);
    } else 
	{
        printf("\n" BOLD CYN
        "  +--------------------------------------------------------------------------+\n"
        "  |ID| NAME                 | FACTION               |  HP | ATK| DEF| SKL| EN|\n"
        "  |--+----------------------+-----------------------+-----+----+----+----+---|\n" RST);
    }
    for(int i = 0; i < 12; i++)
	{
        Champion *c = &champions_catalogue[i];
        if(i > 0 && c->faction != champions_catalogue[i-1].faction)
            printf(DIM "  |--+----------------------+-----------------------+-----+----+----+----+---|\n" RST);
        printf("  |%s%2d%s| %-21s| %s%-22s%s| %3d |%3.0f |%3.0f |%3.0f | %d |\n",
            get_faction_color(c->faction), c->id, RST,
            get_name(c),
            get_faction_color(c->faction), get_faction_name(c->faction), RST,
            c->hp_max, c->attack, c->defense, c->skill, c->energy_max);
    }
    printf(BOLD CYN "  +--------------------------------------------------------------------------+\n\n" RST);
}

void display_skills(Champion *c) 
{
    const char *color = get_faction_color(c->faction);
    if(current_language == 0) printf(BOLD "  Habilidades -- %s%s%s (EN:%d/%d)\n" RST, color, get_name(c), RST, c->energy_current, c->energy_max);
    else                      printf(BOLD "  Skills -- %s%s%s (EN:%d/%d)\n" RST,      color, get_name(c), RST, c->energy_current, c->energy_max);
    for(int i = 0; i < 3; i++)
	{
        Skill *s = &c->skills[i];
        const char *ec = (c->energy_current >= s->energy_cost) ? GRN : RED;
        printf("  %s[%d]%s %-20s %s(EN:%d)%s\n      " DIM "%s\n" RST,
               YLW, i+1, RST,
               (current_language == 0) ? s->name_es : s->name_en,
               ec, s->energy_cost, RST,
               (current_language == 0) ? s->desc_es : s->desc_en);
    }
    if(current_language == 0) printf("  " DIM "[0] Ataque Basico (EN:0)\n" RST);
    else                      printf("  " DIM "[0] Basic Attack (EN:0)\n" RST);
}

void display_faction_cards(Faction f) 
{
    BuffCard *cards = cards_table[(int)f];
    if(current_language == 0) printf(BOLD "  Cartas de Mejora -- %s%s%s\n" RST, get_faction_color(f), get_faction_name(f), RST);
    else                      printf(BOLD "  Buff Cards -- %s%s%s\n" RST,        get_faction_color(f), get_faction_name(f), RST);
    for(int i = 0; i < 4; i++)
	{
        BuffCard *card = &cards[i];
        printf("  " YLW "[%d]" RST " %-22s | %s\n", i+1,
               (current_language == 0) ? card->name_es : card->name_en,
               (current_language == 0) ? card->desc_es : card->desc_en);
    }
    if(current_language == 0) printf("  " DIM "[0] Ninguna carta\n" RST);
    else                      printf("  " DIM "[0] No card\n" RST);
}

/* --------------------------------------
 *  TEAM CONSTRUCTION MODULE
 * -------------------------------------- */
int check_already_chosen(int *chosen, int size, int id) 
{
    for(int i = 0; i < size; i++) if(chosen[i] == id) return 1;
    return 0;
}

void create_team_manual(Team *t, int player_num) 
{
    int chosen[5], count = 0;
    if(current_language == 0)
	{
        printf(BOLD "\n  +----------------------------------+\n"
                    "  |  JUGADOR %d -- ELIGE TU EQUIPO   |\n"
                    "  +----------------------------------+\n\n" RST, player_num);
        printf("  Nombre del equipo: ");
    } else 
	{
        printf(BOLD "\n  +----------------------------------+\n"
                    "  |  PLAYER %d -- BUILD YOUR TEAM    |\n"
                    "  +----------------------------------+\n\n" RST, player_num);
        printf("  Team name: ");
    }
    scanf(" %29[^\n]", t->name);
    if(current_language == 0) printf("\n  Selecciona 5 campeones (IDs 1-12):\n\n");
    else                      printf("\n  Select 5 champions (IDs 1-12):\n\n");

    while(count < 5)
	{
        int id;
        if(current_language == 0) printf("  Campeon %d/5 -- ID: ", count+1);
        else                      printf("  Champion %d/5 -- ID: ", count+1);
        if(scanf("%d", &id) != 1)
		{ clear_input_buffer(); continue; }
        if(id < 1 || id > 12)
		{
            printf(RED "  x %s\n" RST, (current_language==0)?"ID invalido (1-12).":"Invalid ID (1-12).");
            continue;
        }
        if(check_already_chosen(chosen, count, id))
		{
            printf(RED "  x %s\n" RST, (current_language==0)?"Ya elegido.":"Already selected.");
            continue;
        }
        chosen[count] = id;
        t->members[count] = champions_catalogue[id-1];
        for(int s = 0; s < 3; s++) t->members[count].skills[s] = skills_table[id-1][s];
        printf(GRN "  + %s%s%s (%s) -- %s\n" RST,
               get_faction_color(t->members[count].faction), get_name(&t->members[count]), RST,
               get_faction_name(t->members[count].faction), get_role_name(t->members[count].role));
        count++;
    }
    t->count = 5; t->total_team_damage = 0; t->casualties = 0;
}

void create_team_cpu(Team *t) 
{
    if(current_language == 0) strncpy(t->name, "Equipo CPU", 29);
    else                      strncpy(t->name, "CPU Team", 29);
    int chosen[5], count = 0;
    while(count < 5)
	{
        int id = (rand() % 12) + 1;
        if(!check_already_chosen(chosen, count, id))
		{
            chosen[count] = id;
            t->members[count] = champions_catalogue[id-1];
            for(int s = 0; s < 3; s++) t->members[count].skills[s] = skills_table[id-1][s];
            count++;
        }
    }
    t->count = 5; t->total_team_damage = 0; t->casualties = 0;
    printf(YLW "\n  [CPU] %s \"%s\"\n" RST, (current_language==0)?"Equipo generado:":"Random team:", t->name);
}

/* --------------------------------------
 *  MODULE: STATS APPLICATION
 * -------------------------------------- */
void apply_card(Champion *c, BuffCard *card) 
{
    c->attack  *= card->mod_attack;
    c->defense *= card->mod_defense;
    c->skill   *= card->mod_skill;
    int delta_hp = (int)(c->hp_max * (card->mod_hp - 1.0f));
    c->hp_max     += delta_hp;
    c->hp_current += delta_hp;
    if(c->hp_current < 1)         c->hp_current = 1;
    if(c->hp_current > c->hp_max) c->hp_current = c->hp_max;
    c->buff_active = 1;
}

void modify_attribute(Champion *c, int stat_index, float percentage, int is_buff) 
{
    float factor = is_buff ? (1.0f + percentage) : (1.0f - percentage);
    if(stat_index == 0)      c->attack  *= factor;
    else if(stat_index == 1) c->defense *= factor;
    else if(stat_index == 2) c->skill   *= factor;
}

/* --------------------------------------
 *  CORE COMBAT ENGINE
 * -------------------------------------- */
int verify_advantage(Faction attacker, Faction defender) 
{
    if(attacker == WARDENS    && defender == DISSONANTS) return 1;
    if(attacker == DISSONANTS && defender == SYNDICATE)  return 1;
    if(attacker == SYNDICATE  && defender == ARCHITECTS) return 1;
    if(attacker == ARCHITECTS && defender == WARDENS)    return 1;
    return 0;
}
int has_alive_members(Team *t) 
{
    for(int i = 0; i < t->count; i++) if(t->members[i].is_alive) return 1;
    return 0;
}
Champion *get_next_alive_member(Team *t) 
{
    for(int i = 0; i < t->count; i++) if(t->members[i].is_alive) return &t->members[i];
    return NULL;
}

void execute_attack(Champion *attacker, Champion *defender, Team *def_team, Skill *s) 
{
    float a = attacker->attack / 10.0f;
    float base_power = (2.0f * a * a) + (3.0f * a) + 5.0f;
    float damage = base_power * s->damage_multiplier;

    if(s->damage_multiplier >= 2.0f && (strcmp(s->name_en, "Execution") == 0))
        if((defender->hp_current * 100 / defender->hp_max) >= 30)
            damage = base_power * 1.0f;

    modify_attribute(attacker, s->buff_stat, s->buff_percentage, 1);
    modify_attribute(attacker, s->nerf_stat, s->nerf_percentage, 0);

    float multiplier = 1.0f;
    if(verify_advantage(attacker->faction, defender->faction))
        multiplier = (defender->faction == WARDENS && defender->buff_active) ? 1.20f : 1.50f;
    damage *= multiplier;

    float effective_def = (attacker->faction == SYNDICATE && attacker->buff_active)
                          ? (defender->defense * 0.80f) : defender->defense;
    float mitigation = effective_def - (attacker->skill * 0.1f);
    if(mitigation < 0.0f) mitigation = 0.0f;

    printf("\n  %s%-22s%s --> %s%-22s%s\n",
           get_faction_color(attacker->faction), get_name(attacker), RST,
           get_faction_color(defender->faction), get_name(defender), RST);
    printf("  " YLW "[%s]" RST " %s\n",
           (current_language==0)?s->name_es:s->name_en,
           (current_language==0)?s->desc_es:s->desc_en);

    if(multiplier > 1.0f)
        printf("  " YLW "  * %s x%.2f" RST "\n", (current_language==0)?"VENTAJA":"ADVANTAGE", multiplier);
    if(attacker->faction == SYNDICATE && attacker->buff_active)
        printf("  " WHT "  * DEF PENETRATION -20%%" RST "\n");

    if(s->healing_percentage != 0)
	{
        int delta = (int)(attacker->hp_max * (s->healing_percentage / 100.0f));
        if(s->healing_percentage > 0)
		{
            attacker->hp_current += delta;
            if(attacker->hp_current > attacker->hp_max) attacker->hp_current = attacker->hp_max;
            printf("  " GRN "  + %d HP  %d/%d" RST "\n", delta, attacker->hp_current, attacker->hp_max);
        } else {
            attacker->hp_current += delta;
            if(attacker->hp_current < 1) attacker->hp_current = 1;
            printf("  " RED "  Recoil %d HP  %d/%d" RST "\n", delta, attacker->hp_current, attacker->hp_max);
        }
    }

    int hp_impact = 0;
    if(s->damage_multiplier == 0.0f)
	{
        printf("  " CYN "  [%s]" RST "\n", (current_language==0)?"Habilidad sin dano":"No-damage skill");
    } else if(damage <= mitigation)
	{
        defender->defense -= damage;
        if(defender->defense < 0.0f) defender->defense = 0.0f;
        printf("  " CYN "  [%s %.0f | DEF %.1f]" RST "\n",
               (current_language==0)?"Escudo absorbe":"Shield absorbs", damage, defender->defense);
    } else {
        hp_impact = (int)(damage - mitigation);
        int prev = defender->hp_current;
        defender->defense = 0.0f;
        defender->hp_current -= hp_impact;
        if(defender->hp_current < 0) defender->hp_current = 0;
        printf("  " RED "  [HP %d -> %d  (--%d)]" RST "\n", prev, defender->hp_current, hp_impact);
    }

    attacker->total_damage_dealt += hp_impact;
    def_team->total_team_damage  += hp_impact;

    if(s->status_probability > 0 && defender->hp_current > 0)
	{
        if((rand() % 100) < s->status_probability)
		{
            const char *sname = "";
            if(s->apply_paralysis){ defender->paralyzed = 1; sname = (current_language==0)?"PARALISIS":"PARALYSIS"; }
            if(s->apply_poison)   { defender->poisoned  = 1; sname = (current_language==0)?"VENENO":"POISON"; }
            if(s->apply_fear)     { defender->feared    = 1; sname = (current_language==0)?"MIEDO":"FEAR"; }
            if(s->apply_slow)     { defender->slowed    = 1; sname = (current_language==0)?"LENTITUD":"SLOW"; }
            printf("  " MAG "  * %s %s" RST "\n", sname, (current_language==0)?"aplicado":"applied");
        }
    }

    if(!s->is_permanent)
	{
        modify_attribute(attacker, s->buff_stat, s->buff_percentage, 0);
        modify_attribute(attacker, s->nerf_stat, s->nerf_percentage, 1);
    }

    if(defender->poisoned && defender->hp_current > 0)
	{
        int tick = (int)(defender->hp_max * 0.05f);
        defender->hp_current -= tick;
        if(defender->hp_current < 0) defender->hp_current = 0;
        printf("  " GRN "  Poison -%d" RST "\n", tick);
    }

    if(defender->hp_current <= 0)
	{
        defender->is_alive = 0;
        def_team->casualties++;
        printf("  " BOLD RED "  !! %s %s" RST "\n",
               get_name(defender), (current_language==0)?"derrotado.":"defeated.");
        if(defender->faction == ARCHITECTS && defender->buff_active)
		{
            Champion *ally = get_next_alive_member(def_team);
            if(ally != NULL)
			{
                int legacy = (int)(defender->hp_max * 0.15f);
                ally->hp_current += legacy;
                if(ally->hp_current > ally->hp_max) ally->hp_current = ally->hp_max;
                printf("  " MAG "  * ANCESTRAL ECHO: %s +%d HP" RST "\n", get_name(ally), legacy);
            }
        }
    }
}

int resolve_statuses(Champion *c) 
{
    if(c->paralyzed)
	{
        c->paralyzed = 0;
        printf("  " YLW "  * %s %s" RST "\n", get_name(c),
               (current_language==0)?"PARALIZADO -- pierde turno":"PARALYZED -- turn lost");
        return 1;
    }
    if(c->feared)
	{
        c->feared = 0;
        if((rand() % 100) < 50)
		{
            printf("  " YLW "  * %s %s" RST "\n", get_name(c),
                   (current_language==0)?"ASUSTADO -- pierde turno":"FEARED -- turn lost");
            return 1;
        }
    }
    if(c->slowed)
	{
        c->slowed = 0;
        printf("  " DIM "  * %s SLOW (Fin de penalizacion)\n" RST, get_name(c));
    }
    return 0;
}

void display_fighter_state(Champion *c, const char *label) 
{
    const char *color = get_faction_color(c->faction);
    printf("  %s%-6s %-22s%s %s\n", color, get_faction_icon(c->faction), get_name(c), RST, label);
    printf("    HP  "); print_hp_bar(c->hp_current, c->hp_max, color); printf("\n");
    printf("    EN  "); print_energy_bar(c->energy_current, c->energy_max, color); printf("\n");
    printf("    " DIM "ATK:%-6.1f DEF:%-6.1f SKL:%-6.1f" RST "\n", c->attack, c->defense, c->skill);
}

Skill *menu_choose_skill(Champion *attacker) 
{
    display_skills(attacker);
    printf("\n  %s (0-3): ", (current_language==0)?"Elige habilidad":"Choose skill");
    int choice;
    while(1)
	{
        if(scanf("%d", &choice) != 1)
		{ clear_input_buffer(); continue; }
        if(choice >= 0 && choice <= 3) break;
        printf("  " RED "x" RST " 0-3: ");
    }
    if(choice == 0) return NULL;
    Skill *s = &attacker->skills[choice-1];
    if(attacker->energy_current < s->energy_cost)
	{
        printf(RED "  Energia insuficiente. %s.\n" RST,
               (current_language==0)?"Ataque basico automatico":"Basic attack forced");
        return NULL;
    }
    attacker->energy_current -= s->energy_cost;
    return s;
}

static Skill global_basic_attack = 
{
    "Ataque Basico","Basic Attack",
    "Ataque estandar, sin coste de energia.","Standard attack, no energy cost.",
    1.0f,0.0f,3,0.0f,3,0,0,0,0,0,0,0,0
};

Skill *cpu_choose_skill(Champion *attacker) 
{
    Skill *best = NULL; float highest = 0.0f;
    for(int i = 0; i < 3; i++)
	{
        Skill *s = &attacker->skills[i];
        if(attacker->energy_current >= s->energy_cost && s->damage_multiplier > highest){
            highest = s->damage_multiplier; best = s;
        }
    }
    if(best != NULL)
	{
        printf(DIM "  [CPU] %s: %s\n" RST, get_name(attacker),
               (current_language==0)?best->name_es:best->name_en);
        attacker->energy_current -= best->energy_cost;
    }
    return best;
}

void menu_choose_card(Champion *attacker) 
{
    display_faction_cards(attacker->faction);
    printf("\n  %s (0-4): ", (current_language==0)?"Carta de mejora":"Buff card");
    int choice;
    while(1)
	{
        if(scanf("%d", &choice) != 1)
		{ clear_input_buffer(); continue; }
        if(choice >= 0 && choice <= 4) break;
        printf("  " RED "x" RST " 0-4: ");
    }
    if(choice == 0) return;
    BuffCard *card = &cards_table[(int)attacker->faction][choice-1];
    apply_card(attacker, card);
    printf(YLW "  + %s: %s\n" RST,
           (current_language==0)?"Carta aplicada":"Card applied",
           (current_language==0)?card->name_es:card->name_en);
}

void cpu_choose_card(Champion *attacker) 
{
    if((rand() % 100) < 40)
	{
        int index = rand() % 4;
        BuffCard *card = &cards_table[(int)attacker->faction][index];
        apply_card(attacker, card);
        printf(DIM "  [CPU] Card: %s\n" RST,
               (current_language==0)?card->name_es:card->name_en);
    }
}

void start_combat(Team *team_a, Team *team_b, GameMode mode) 
{
    int round_counter = 1, active_team_turn = 0;
    printf(BOLD RED
           "\n  +--------------------------------------------------+\n"
           "  |  Local: %-16s VS  Visita: %-16s |\n"
           "  +--------------------------------------------------+\n\n" RST,
           team_a->name, team_b->name);

    while(has_alive_members(team_a) && has_alive_members(team_b))
	{
        Champion *attacker, *defender;
        Team *def_team;
        int is_cpu;

        if(active_team_turn == 0)
		{
            attacker = get_next_alive_member(team_a);
            defender = get_next_alive_member(team_b);
            def_team = team_b; is_cpu = 0;
        } else 
		{
            attacker = get_next_alive_member(team_b);
            defender = get_next_alive_member(team_a);
            def_team = team_a; is_cpu = (mode == MODE_PVE) ? 1 : 0;
        }
        if(attacker == NULL || defender == NULL) break;

        print_double_divider();
        printf(BOLD "\n  %s %d -- %s%s%s %s\n" RST,
               (current_language==0)?"TURNO":"TURN", round_counter,
               get_faction_color(attacker->faction), get_name(attacker), RST,
               is_cpu ? "[CPU]" : ((current_language==0)?"[JUGADOR]":"[PLAYER]"));
        print_double_divider();

        display_fighter_state(attacker, (current_language==0)?">> ACTIVO":">> ACTIVE");
        printf("\n");
        display_fighter_state(defender, (current_language==0)?"<< ENEMIGO":"<< ENEMY");

        if(resolve_statuses(attacker))
		{
            attacker->energy_current++;
            if(attacker->energy_current > attacker->energy_max) attacker->energy_current = attacker->energy_max;
            active_team_turn = 1 - active_team_turn; round_counter++; continue;
        }

        attacker->energy_current++;
        if(attacker->energy_current > attacker->energy_max) attacker->energy_current = attacker->energy_max;

        printf("\n");
        if(is_cpu) cpu_choose_card(attacker);
        else       menu_choose_card(attacker);

        Skill *chosen_skill;
        if(is_cpu) chosen_skill = cpu_choose_skill(attacker);
        else       chosen_skill = menu_choose_skill(attacker);

        if(chosen_skill == NULL) chosen_skill = &global_basic_attack;
        execute_attack(attacker, defender, def_team, chosen_skill);

        if(!defender->is_alive)
		{
            Champion *backup = get_next_alive_member(def_team);
            if(backup != NULL)
                printf("  %s  >> %s %s\n\n", get_faction_color(backup->faction),
                       get_name(backup), (current_language==0)?"entra al campo.":"enters the field.");
        }

        active_team_turn = 1 - active_team_turn; round_counter++;
        if(round_counter > 300) break;
    }
}

/* --------------------------------------
 *  MODULE: STATISTICS ENGINE
 * -------------------------------------- */
void display_final_statistics(Team *team_a, Team *team_b) 
{
    printf("\n" BOLD YLW
    "  +----------------------------------------------+\n"
    "  |             ESTADISTICAS FINALES             |\n"
    "  +----------------------------------------------+\n\n" RST);

    int alive_a = 0, alive_b = 0;
    for(int i = 0; i < 5; i++)
	{
        if(team_a->members[i].is_alive) alive_a++;
        if(team_b->members[i].is_alive) alive_b++;
    }

    if(alive_a > 0)      printf(BOLD GRN "  ** %s: %s\n\n" RST, (current_language==0)?"GANADOR":"WINNER", team_a->name);
    else if(alive_b > 0) printf(BOLD GRN "  ** %s: %s\n\n" RST, (current_language==0)?"GANADOR":"WINNER", team_b->name);
    else                 printf(BOLD YLW "  ** %s\n\n" RST,      (current_language==0)?"EMPATE":"DRAW");

    printf(BOLD "  %-26s %-26s\n" RST, team_a->name, team_b->name);
    print_divider();
    printf("  %s: " GRN "%d" RST "  /  %s: " GRN "%d" RST "\n",
           (current_language==0)?"Dano total":"Total DMG", team_a->total_team_damage,
           (current_language==0)?"Dano total":"Total DMG", team_b->total_team_damage);
    printf("  %s: " RED "%d" RST "  /  %s: " RED "%d" RST "\n",
           (current_language==0)?"Bajas":"Casualties", team_a->casualties,
           (current_language==0)?"Bajas":"Casualties", team_b->casualties);
    print_divider();

    Champion *mvp = &team_a->members[0];
    for(int i = 0; i < 5; i++)
	{
        if(team_a->members[i].total_damage_dealt > mvp->total_damage_dealt) mvp = &team_a->members[i];
        if(team_b->members[i].total_damage_dealt > mvp->total_damage_dealt) mvp = &team_b->members[i];
    }
    printf(BOLD YLW "\n  >> MVP: %s%s%s -- %d DMG\n" RST,
           get_faction_color(mvp->faction), get_name(mvp), RST, mvp->total_damage_dealt);
    print_double_divider();
}

/* --------------------------------------
 *  MAIN CONTROLLER
 * -------------------------------------- */
int main(void) 
{
    enable_ansi();
    srand((unsigned int)time(NULL));

    Team team_1, team_2;
    memset(&team_1, 0, sizeof(Team));
    memset(&team_2, 0, sizeof(Team));

    int menu_option;
    int teams_ready = 0, combat_completed = 0;
    GameMode current_mode = MODE_PVP;

    display_title_screen();
    press_to_continue();

    do {
        display_title_screen();
        printf(BOLD
        "  +--------------------------------+\n"
        "  |   %-29s|\n"
        "  |--------------------------------|\n"
        "  |  %-30s|\n"
        "  |  %-30s|\n"
        "  |  %-30s|\n"
        "  |  %-30s|\n"
        "  |  %-30s|\n"
        "  |  %-30s|\n"
        "  |  %-30s|\n"
        "  +--------------------------------+\n\n" RST,
        (current_language==0)?"MENU PRINCIPAL":"MAIN MENU",
        (current_language==0)?"1. Ver Catalogo":"1. View Catalogue",
        (current_language==0)?"2. Modo (PvP/PvE)":"2. Mode (PvP/PvE)",
        (current_language==0)?"3. Formar Equipos":"3. Build Teams",
        (current_language==0)?"4. Ejecutar Batalla":"4. Run Battle",
        (current_language==0)?"5. Estadisticas":"5. Final Stats",
        (current_language==0)?"6. Cambiar Idioma":"6. Toggle Language",
        (current_language==0)?"7. Salir":"7. Exit");

        printf("  Modo: %s%s" RST " | %s: [%s]  %s: [%s]\n\n",
               YLW, (current_mode == MODE_PVP) ? "PvP" : "PvE",
               (current_language==0)?"Equipos":"Teams",
               teams_ready ? GRN "OK" RST : RED "--" RST,
               (current_language==0)?"Batalla":"Battle",
               combat_completed ? GRN "OK" RST : RED "--" RST);

        printf("  %s", (current_language==0)?"Opcion: ":"Option: ");
        if(scanf("%d", &menu_option) != 1){ clear_input_buffer(); continue; }
        clear_input_buffer();

        switch(menu_option)
		{
            case 1:
                display_catalogue();
                press_to_continue(); break;
            case 2:
                printf(BOLD "\n  %s\n" RST, (current_language==0)?"MODO DE JUEGO":"GAME MODE");
                printf("  1. PvP -- %s\n", (current_language==0)?"Jugador vs Jugador":"Player vs Player");
                printf("  2. PvE -- %s\n", (current_language==0)?"Jugador vs CPU":"Player vs CPU");
                printf("  %s", (current_language==0)?"Opcion: ":"Option: ");
                { int m; if(scanf("%d", &m) != 1){ clear_input_buffer(); break; }
                  clear_input_buffer();
                  current_mode = (m == 2) ? MODE_PVE : MODE_PVP;
                  teams_ready = 0; combat_completed = 0; }
                break;
            case 3:
                display_catalogue();
                create_team_manual(&team_1, 1);
                if(current_mode == MODE_PVP) create_team_manual(&team_2, 2);
                else                         create_team_cpu(&team_2);
                teams_ready = 1; combat_completed = 0;
                press_to_continue(); break;
            case 4:
                if(!teams_ready)
				{
                    printf(RED "  x %s\n" RST, (current_language==0)?
                           "Forma los equipos primero (Opcion 3).":
                           "Build teams first (Option 3).");
                    press_to_continue(); break;
                }
                start_combat(&team_1, &team_2, current_mode);
                combat_completed = 1;
                press_to_continue(); break;
            case 5:
                if(!combat_completed)
				{
                    printf(RED "  x %s\n" RST, (current_language==0)?
                           "Ejecuta la batalla primero (Opcion 4).":
                           "Run battle first (Option 4).");
                    press_to_continue(); break;
                }
                display_final_statistics(&team_1, &team_2);
                press_to_continue(); break;
            case 6:
                current_language = (current_language == 0) ? 1 : 0;
                break;
            case 7:
                printf(DIM "\n  // Desconectando frecuencia...\n\n" RST);
                break;
            default:
                printf(RED "  x %s\n" RST, (current_language==0)?"Opcion invalida.":"Invalid option.");
                press_to_continue();
        }
    } while(menu_option != 7);

    return 0;
}