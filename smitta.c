#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

struct person {
	/* En räknare, när smittad>0 är personen sjuk. Siffran är antalet kvarvarande dagar */
	int			smittad;
	
	/* Antal dagar innan personen kan smitta andra */
	int			cooldown;
	
	/* Flagga som markerar immunitet */
	int			immun;

	/* Flagga för död person */
	int			doed;
};


/* Matrisen representerar befolkningen i experimentet */
struct {
	struct person		*person;
	int			person_w;
	int			person_h;

	int			smitta_t_min;
	int			smitta_t_max;
	float			smitta_risk;
	float			doedsrisk;

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
	/* Slumpa fram om personen dör eller inte */
	if (Matris.doedsrisk < slumptal())
		return;
	/* Personen dör */
	Matris.person[index_t(x, y)].doed = 1;

	/* Uppdatera statistik */
	Matris.doeda_idag++;
	Matris.doeda_totalt++;
	return;
}


/* Försöker smitta en närliggande person */
void smitta_kanske(int x, int y) {
	float n;
	int dx, dy, t, s;

	n = slumptal();
	/* Smitta endast om personen har sådan otur */
	if (Matris.smitta_risk < n)
		return;
	
	do {
		/* Slumpa fram en närliggande cell att smitta till */
		dx = rand() % 3 - 1;
		dy = rand() % 3 - 1;
	} while (!dx && !dy); /* Personen kan inte smitta sig själv */

	if ((t = index_t(x + dx, y + dy)) < 0) {
		/* Cellen är utanför kanterna, blir ingen smitta */
		return;
	}

	/* Personen har redan smittats */
	if (Matris.person[t].immun)
		return;

	/* slumpa fram antal dagar s som personen kommer vara sjuk */
	s = rand() % (Matris.smitta_t_max - Matris.smitta_t_min + 1) + Matris.smitta_t_min;
	Matris.person[t].smittad = s;
	Matris.person[t].cooldown = 1;
	Matris.person[t].immun = 1;

	/* Uppdatera smittostatistik */
	Matris.smittade_idag++;
	Matris.smittade_totalt++;

	return;
}


/* Simulerar en dag */
void simulera_dag() {
	int i, j;

	/* Rensa dagsstaistiken */
	Matris.friska_idag = Matris.sjuka_idag = Matris.doeda_idag = Matris.smittade_idag = 0;

	for (i = 0; i < Matris.person_h; i++)
		for (j = 0; j < Matris.person_w; j++) {
			Matris.person[index_t(j, i)].smittad--;
			Matris.person[index_t(j, i)].cooldown--;
			/* Smittad och smittande? */
			if (Matris.person[index_t(j, i)].smittad > 0 && Matris.person[index_t(j, i)].cooldown < 1 && !Matris.person[index_t(j, i)].doed) {
				smitta_kanske(j, i);
				/* Singla slant om personen ska dö eller inte */
				doeda_kanske(j, i);
			}

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
	int paus = 0, n, x, y, t;
	srand(time(NULL));

	if (argc < 7) {
		fprintf(stderr, "Usage: %s <individmatrisstorlek (N×N)> <smittotid minimum> <smittotid maximum> <smittorisk [0,1]> <dödsrisk [0,1]> <långsam simulering>\n", argv[0]);
		return -1;
	}
	n = atoi(argv[1]);
	Matris.smitta_t_min = atoi(argv[2]);
	Matris.smitta_t_max = atoi(argv[3]);
	sscanf(argv[4], "%f", &Matris.smitta_risk);
	sscanf(argv[5], "%f", &Matris.doedsrisk);
	paus = atoi(argv[6]);

	matris_init(n);

	fprintf(stderr, "Ange koordinater for nysmittade. Markera att du är färdig genom att ange -1 som X-koordinat\n");
	do {
		x = -1;
		fprintf(stderr, "X-koordinat i %i×%i-matris att placera smittad: ", n, n);
		fscanf(stdin, "%i", &x);
		if (x < 0)
			break;
		fprintf(stderr, "Y-koordinat i %i×%i-matris att placera smittad: ", n, n);
		fscanf(stdin, "%i", &y);
		t = index_t(x, y);
		if (t < 0) {
			fprintf(stderr, "Ogiltig koordinat!\n");
			continue;
		}

		fprintf(stderr, "Dagar personen kommer vara sjuk: ");
		fscanf(stdin, "%i", &Matris.person[t].smittad);
		Matris.person[t].immun = 1;
		Matris.person[t].cooldown = 1;
		Matris.smittade_totalt++;
	} while (1);

	do {
		simulera_dag();
		i++;

		/* Skriv ut statistik */
		fprintf(stdout, "Antal insjuknade i dag    : %.5i\n", Matris.smittade_idag);
		fprintf(stdout, "Antal tillfrisknade i dag : %.5i\n", Matris.friska_idag);
		fprintf(stdout, "Antal sjuka totalt i dag  : %.5i\n", Matris.sjuka_idag);
		fprintf(stdout, "Antal smittade totalt     : %.5i\n", Matris.smittade_totalt);
		fprintf(stdout, "Antal dödsfall i dag      : %.5i\n", Matris.doeda_idag);
		fprintf(stdout, "Antal dödsfall totalt     : %.5i\n", Matris.doeda_totalt);
		fprintf(stdout, "---------------------------------\n");
	
		/* Om simuleringen skall köra en dag varannan sekund, pausa en stund */
		if (paus)
			sleep(2);
	} while (Matris.sjuka_idag > 0);

	fprintf(stdout, "Antalet sjuka är nu noll, stoppar simulering. Det har gått %i dag(ar)\n", i);
	fprintf(stdout, "Totalt smittades %i individer i populationen (%i %%)\n", Matris.smittade_totalt, Matris.smittade_totalt * 100 / (Matris.person_w * Matris.person_h));

	return 0;
}
