#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct person {
	/* En räknare, när smittad>0 är personen sjuk. Siffran är antalet kvarvarande dagar */
	int			smittad;
	
	/* Antal dagar innan personen kan smitta andra */
	int			cooldown;
	
	/* Flagga som markerar immunitet */
	int			immun;
};


/* Matrisen representerar befolkningen i experimentet */
struct {
	struct person		*person;
	int			person_w;
	int			person_h;

	int			smitta_t_min;
	int			smitta_t_max;
	float			smitta_risk;

	int			friska_idag;
	int			sjuka_idag;
	int			smittade_idag;
	int			doeda_idag;
	int			doeda_totalt;
	int			smittade_totalt;
} Matris;


float slumptal() {
	float n;
	
	/* Generera ett slumptal och skala om till en float i intervall 0..1 */
	n = ((float) (rand() & 0xFFFF)) / 65536;
	return n;
}


/* Bredd på NxN-matrisen */
void matris_init(int bredd) {
	Matris.person = calloc(sizeof(*Matris.person) * bredd * bredd, 1);
	Matris.person_w = Matris.person_h = bredd;
	Matris.doeda_totalt = 0;
	return;
}


int index_t(int x, int y) {
	if (x < 0 || x >= Matris.person_w)
		return -1;
	if (y < 0 || y >= Matris.person_h)
		return -1;
	return x + y * Matris.person_w;
}


void doeda_kanske(int x, int y) {
	/* I den här simuleringen dödar vi inte något */
	return;
}


void smitta_kanske(int x, int y) {
	float n;
	int dx, dy, t, s;

	n = slumptal();
	/* Smitta endast om personen har sådan otur */
	if (Matris.smitta_risk > n)
		return;
	/* Slumpa fram en närliggande cell att smitta till */
	dx = rand() % 3 - 1;
	dy = rand() % 3 - 1;

	if ((t = index_t(x + dx, y + dy)) < 0) {
		/* Cellen är utanför kanterna, blir ingen smitta */
		return;
	}

	/* slumpa fram antal dagar s som personen kommer vara sjuk */
	s = rand() % (Matris.smitta_t_max - Matris.smitta_t_min);
	Matris.person[t].smittad = s;
	Matris.person[t].cooldown = 1;
	Matris.person[t].immun = 1;

	/* Uppdatera smittostatistik */
	Matris.smittade_idag++;
	Matris.smittade_totalt++;

	return;
}


void simulera_dag() {
	int i, j;

	/* Rensa dagsstaistiken */
	Matris.friska_idag = Matris.sjuka_idag = Matris.doeda_idag = Matris.smittade_idag = 0;

	for (i = 0; i < Matris.person_h; i++)
		for (j = 0; j < Matris.person_w; j++) {
			Matris.person[index_t(j, i)].smittad--;
			Matris.person[index_t(j, i)].cooldown--;
			/* Smittad och smittande? */
			if (Matris.person[index_t(j, i)].smittad > 0 && Matris.person[index_t(j, i)].cooldown < 1)
				smitta_kanske(j, i);
			/* Singla slant om personen ska dö eller inte */
			doeda_kanske(j, i);

			/* Personen blev frisk i dag */
			if (!Matris.person[index_t(j, i)].smittad)
				Matris.friska_idag++;
			else if (Matris.person[index_t(j, i)].smittad > 0)
				Matris.sjuka_idag++;

			/* TODO: skriv ut cell */
		}
	return;
}

			


int main(int argc, char **argv) {
	int i = 0;
	srand(time(NULL));
	matris_init(50);

	do {
		simulera_dag();
		i++;
	} while (Matris.sjuka_idag > 0);

	fprintf(stdout, "Antalet sjuka är nu noll, stoppar simulering. Det har gått %i dag(ar)\n", i);
	fprintf(stdout, "Totalt smittades %i av populationen (%i %%)\n", Matris.smittade_totalt, Matris.smittade_totalt * 100 / (Matris.person_w * Matris.person_h));

	return 0;
}
