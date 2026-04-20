#include "include/getopt.h"
#include "include/string.h"

/* ------------------------------------------------------------------ */
/* Variáveis globais de estado                                         */
/* ------------------------------------------------------------------ */

char *optarg = (void *)0;
int   optind = 1;
int   opterr = 1;
int   optopt = 0;

/* Posição dentro do token atual (ex: -abc → processa a, b, c em ordem) */
static const char *_next = (void *)0;

/* ------------------------------------------------------------------ */
/* Helpers internos                                                    */
/* ------------------------------------------------------------------ */

/** Localiza opt em optstring; retorna ponteiro para o char encontrado ou NULL. */
static const char *_find_short(const char *optstring, int opt) {
    const char *p = optstring;
    if (*p == ':') p++; /* pula indicador de modo silencioso */
    while (*p != '\0') {
        if ((unsigned char)*p == opt) return p;
        p++;
    }
    return (void *)0;
}

/* ------------------------------------------------------------------ */
/* getopt — opções curtas                                              */
/* ------------------------------------------------------------------ */

int getopt(int argc, char * const argv[], const char *optstring) {
    int silent = (optstring[0] == ':');

    if (optind >= argc)
        return -1;

    /* Avança para o próximo token de opção */
    if (_next == (void *)0 || *_next == '\0') {
        const char *arg = argv[optind];

        if (arg[0] != '-' || arg[1] == '\0')
            return -1;

        if (arg[1] == '-' && arg[2] == '\0') { /* "--" */
            optind++;
            return -1;
        }

        /* Token começa com "--": não é opção curta */
        if (arg[1] == '-')
            return -1;

        _next = arg + 1;
        optind++;
    }

    int opt = (unsigned char)*_next++;
    const char *p = _find_short(optstring, opt);

    if (p == (void *)0) {
        optopt = opt;
        return '?';
    }

    if (*(p + 1) == ':') {
        if (*_next != '\0') {
            optarg = (char *)_next;
            _next   = (void *)0;
        } else if (optind < argc) {
            optarg = argv[optind++];
            _next   = (void *)0;
        } else {
            optopt = opt;
            _next  = (void *)0;
            return silent ? ':' : '?';
        }
    } else {
        optarg = (void *)0;
    }

    return opt;
}

/* ------------------------------------------------------------------ */
/* getopt_long — opções curtas + longas                                */
/* ------------------------------------------------------------------ */

int getopt_long(int argc, char * const argv[], const char *optstring,
                const option_t *longopts, int *longindex) {

    int silent = (optstring[0] == ':');

    if (optind >= argc)
        return -1;

    /* Se há um token curto em processamento, delega para getopt() */
    if (_next != (void *)0 && *_next != '\0')
        return getopt(argc, argv, optstring);

    const char *arg = argv[optind];

    /* Argumento não é opção */
    if (arg[0] != '-' || arg[1] == '\0')
        return -1;

    /* Separador "--" */
    if (arg[1] == '-' && arg[2] == '\0') {
        optind++;
        return -1;
    }

    /* Opção longa: começa com "--" */
    if (arg[1] == '-') {
        const char *name  = arg + 2;          /* pula "--" */
        const char *eq    = (void *)0;        /* posição do '=' se houver */
        size_t      nlen  = 0;

        /* Separa nome do valor inline: --out=arquivo */
        for (size_t i = 0; name[i] != '\0'; i++) {
            if (name[i] == '=') {
                eq   = name + i;
                nlen = i;
                break;
            }
        }
        if (eq == (void *)0)
            nlen = strlen(name);

        /* Busca em longopts */
        int found = -1;
        for (int i = 0; longopts[i].name != (void *)0; i++) {
            if (strncmp(longopts[i].name, name, nlen) == 0 &&
                longopts[i].name[nlen] == '\0') {
                found = i;
                break;
            }
        }

        optind++;

        if (found < 0) {
            optopt = 0;
            return '?';
        }

        if (longindex != (void *)0)
            *longindex = found;

        const option_t *lo = &longopts[found];

        /* Trata argumento */
        if (lo->has_arg == required_argument || lo->has_arg == optional_argument) {
            if (eq != (void *)0) {
                /* --nome=valor */
                optarg = (char *)(eq + 1);
            } else if (lo->has_arg == required_argument && optind < argc) {
                /* --nome valor */
                optarg = argv[optind++];
            } else if (lo->has_arg == required_argument) {
                /* Argumento obrigatório ausente */
                optopt = 0;
                return silent ? ':' : '?';
            } else {
                optarg = (void *)0; /* optional_argument sem valor */
            }
        } else {
            optarg = (void *)0;
        }

        /* Retorno conforme flag */
        if (lo->flag != (void *)0) {
            *lo->flag = lo->val;
            return 0;
        }
        return lo->val;
    }

    /* Opção curta normal */
    return getopt(argc, argv, optstring);
}
