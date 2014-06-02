#include <stdio.h>
#include <stdlib.h>

struct person {
	/* En räknare, när smittad>0 är personen sjuk. Siffran är antalet kvarvarande dagar */
	int			smittad;
};


/* Matrisen representerar befolkningen i experimentet */
struct {
	struct person		*person;
	int			person_w;
	int			person_h;
} Matris;


/* Bredd på NxN-matrisen */
void matris_init(int bredd) {
	Matris.person = calloc(sizeof(*Matris.person) * bredd * bredd, 1);
	Matris.person_w = Matris.person_h = bredd;
	return;
}


int main(int argc, char **argv) {
	matris_init(50);
	return 0;
}
