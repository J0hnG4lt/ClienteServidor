struct nodo {
    char *identificador;
};

struct conj {
    struct nodo *elemento;
    struct conj *siguiente;
};

void inicializarConj(struct conj *conjunto, char *identificador, time_t tiempo);
int insertarEnConj(struct conj *conjunto, char *identificador, time_t tiempo);
void liberarConj(struct conj *conjunto);
int eliminarEnConj(struct conj **conjunto, char *identificador);
void imprimirConj(struct conj *conjunto);
