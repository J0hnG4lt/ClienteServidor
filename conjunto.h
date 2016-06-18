struct nodo {
    int identificador;
};

struct conj {
    struct nodo *elemento;
    struct conj *siguiente;
};

void inicializarConj(struct conj *conjunto, int identificador);
void insertarEnConj(struct conj *conjunto, int identificador);
void liberarConj(struct conj *conjunto);
int eliminarEnConj(struct conj **conjunto, int identificador);
void imprimirConj(struct conj *conjunto);
