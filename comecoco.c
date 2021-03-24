#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <math.h>
#include <graphics.h>

#define MTX 15				/* Tamaño del mapa */
#define MTY 13
#define MCX 95				/* Posicion mapa en pantalla */
#define MCY 15

#define MAX_ROTULOS 10		/* Nº maximo de rotulos */
#define P_VEL 5	   			/* Velocidad del jugador */
#define INI_FANTASMAS  3    /* Numero de fantasmas inicial */
#define MAX_FANTASMAS  10   /* Numero Maximo de fantasmas */
#define P_FANTASMA	  100	/* Puntos que suma comer un fantasma */
#define P_PILDORA	  5		/* Puntos que suma una pildora */
#define P_SPILDORA	  5		/* Puntos que suma una superpildora */
#define P_VIDAEXTRA	  1000	/* Puntos necesarios para una vida extra */
#define T_PARTIDA	  180	/* Tiempo de partida inicial en segundos */

/* Variables Globales */

typedef struct S_KEKO {		/* Estructura de los kekos */
	int x, y, espera, vel;
	char activo, dir, color;
} S_KEKO;

typedef struct S_ROTULO {
	int x, y, espera, color;
	char texto[10];
} S_ROTULO;

S_KEKO keko[MAX_FANTASMAS+1];
S_ROTULO rotulo[MAX_ROTULOS];
char tecla,					/* Tecla pulsada */
	 mapa[MTY][MTX];		/* Mapa del juego */
int pildoras,				/* Pildoras acumuladas en el nivel */
	tiempo = 0,
	ultimavidaextra,		/* Puntuacion de la ultima vida extra */
	puntos,					/* Puntos totales acumulados */
	vidas,					/* Vidas restantes */
	nivel,					/* Nivel de dificultad */
	pild_total,				/* Pildoras totales en el nivel */
	n_fantasmas,			/* Nº fantasmas en el nivel */
	modo_agresivo;			/* Fantasmas en modo agresivo */
clock_t clck;				/* Reloj */

/* Declaracion de funciones */

void inicializar_juego(void);
void pausa_inicio(void);
void pasa_nivel(void);
void reset_nivel(void);
void control_velocidad(void);
void mover_jugador(void);
void mover_fantasmas(void);
void control_fantasmas(void);
void ia_agresivo(int cont);
void control_jugador();
int comprueba_choque (S_KEKO p);
int comprueba_camino (S_KEKO p, char dir);
int comprueba_casilla (S_KEKO p);
void camino_aleatorio(int cont);
void cuadra_keko (int num, float grid);
int num_pild(void);
int velocidad(void);
void reset_rotulos(void);
void set_rotulo(int x, int y, char texto[10], int tiempo);

void dibuja_rotulos(void);
void dibuja_mapa(void);
void dibuja_pild(void);
void dibuja_puntuacion(void);
void flash(char color);
void partida_ganada(void);
void partida_perdida(void);
void sprite(char id, int x, int y, char color);
void carga_mapa(int mapanivel);

void main() {

	int gdriver = VGA, gmode = VGAHI, cont;
	randomize();

	/* Modo gráfico */
	initgraph(&gdriver, &gmode, "");

	/* Establece la fuente */
	settextstyle(SMALL_FONT, HORIZ_DIR, 4);
	settextjustify(CENTER_TEXT, TOP_TEXT);

	carga_mapa(nivel);
	dibuja_mapa();
	pausa_inicio();
	inicializar_juego();
	dibuja_puntuacion();

	/* Comienza el bucle del juego */
	do {
		dibuja_pild();
		control_fantasmas();
		control_jugador();
		mover_jugador();
		mover_fantasmas();
		dibuja_rotulos();

		if (keko[0].espera > 0) keko[0].espera--; /* Superpildora */
		if (pildoras == pild_total) partida_ganada();
		if (vidas < 0) partida_perdida();

		/* Control reloj */
		if (clck%(int)CLK_TCK==0 && tiempo > 0) {
			tiempo--; /* Reloj Partida */
			modo_agresivo = 0;
			dibuja_puntuacion();
			}
		if (tiempo<1 && modo_agresivo == 0) {
			flash(13);
			dibuja_mapa();
			flash(14);
			dibuja_mapa();
			modo_agresivo = 1;
			tiempo = 0;
			}

		/* Vida extra */
		if (puntos-ultimavidaextra >= P_VIDAEXTRA) {
			vidas++;
			ultimavidaextra += P_VIDAEXTRA;
			dibuja_puntuacion();
			set_rotulo(keko[0].x+14,keko[0].y+8,"1UP",5);
			}

		control_velocidad();
	} while (tecla != 's' && tecla != 'S');

	closegraph(); /* Cierra modo gráfico */

}





/*************/
/* FUNCIONES */
/*************/

/* Inicializa las variables del juego */
void inicializar_juego(void) {
	int cont;
	tecla = NULL;				// Borra tecla pulsada
	clck = clock();				// Inicializa el reloj para control_velocidad
	puntos = 0;					// Inicializa contador puntos.
	pildoras = 0;				// Inicializa contador pildoras
	vidas = 3;					// Establece las vidas de comienzo
	ultimavidaextra=0;
	tiempo = T_PARTIDA;
	nivel = 0;					// Nivel de juego inicial
	modo_agresivo = 0;			// Modo agresivo apagado
	n_fantasmas = INI_FANTASMAS+(nivel/3);	// Nº de fantasmas iniciales
	keko[0].dir = 0;			// Direccion inicial del jugador
	keko[0].espera = 0;			// Tiempo superpildora inicial
	keko[0].x = 7*30;			// Posicion inicial
	keko[0].y = 11*30;
	for (cont=1; cont <= n_fantasmas; cont++) {
		keko[cont].dir = 0;					// Direccion inicial fantasmas
		keko[cont].x = 7*30;				// Posicion inicial
		keko[cont].y = 6*30;
		keko[cont].espera = 32*cont;		// Tiempo espera salida
	}
	reset_rotulos();
}

/* Hace una pausa y espera una tecla */
void pausa_inicio(void) {
	setcolor(15);
	outtextxy(MCX+MTX*15,MCY+MTY*30,"Pulsa una tecla para comenzar");
	getch();
}

/* Reestablece variables y pasa de nivel */
void pasa_nivel(void) {
	int cont;
	tecla = NULL;
	clck = clock();
	nivel++;					// Aumenta nivel dificultad
	tiempo = T_PARTIDA-nivel*5;
	n_fantasmas = INI_FANTASMAS+(nivel/3);	// Nº de fantasmas iniciales
	pildoras = 0;				// Inicializa contador pildoras
	keko[0].dir = 0;			// Direccion inicial del jugador
	keko[0].espera = 0;			// Tiempo superpildora inicial
	keko[0].x = 7*30;			// Posicion inicial
	keko[0].y = 11*30;
	for (cont=1; cont <= n_fantasmas; cont++) {
		keko[cont].dir = 0;					// Direccion Inicial
		keko[cont].x = 7*30;				// Posicion inicial
		keko[cont].y = 6*30;
		keko[cont].espera = 32*cont;		// Tiempo espera salida
	}
	reset_rotulos();
}

/* Resetea las variables al comienzo del nivel */
void reset_nivel(void) {
	int cont;
	tecla = NULL;
	clck = clock();
	tiempo = T_PARTIDA-nivel*5;
	keko[0].dir = 0;			// Direccion jugador inicial
	keko[0].espera = 0;			// Tiempo superpildora inicial
	keko[0].x = 7*30;			// Posicion inicial
	keko[0].y = 11*30;
	for (cont=1; cont <= n_fantasmas; cont++) {
		keko[cont].dir = 0;					// Direccion Inicial
		keko[cont].x = 7*30;				// Posicion Inicial
		keko[cont].y = 6*30;
		keko[cont].espera = 32*cont;		// Tiempo espera salida
	}
}

/* Controla la velocidad del juego */
void control_velocidad(void) {
	while (clck == clock());
	clck = clock();
	}

/* Mueve al jugador y lo situa en pantalla */
void mover_jugador(void) {
	int cont;

	/* Borrado */
	sprite(0, MCX+keko[0].x, MCY+keko[0].y, -1);

	/* Movimiento */
	switch(keko[0].dir) {
		case 1:	keko[0].y-=P_VEL; break;
		case 2:	keko[0].y+=P_VEL; break;
		case 3:	keko[0].x-=P_VEL; break;
		case 4:	keko[0].x+=P_VEL; break;
	}

	/* Dibujo */
	if ((keko[0].x%30 < 15 && keko[0].y%30 == 0) ||
		(keko[0].x%30 == 0 && keko[0].y%30 < 15))
		sprite(18+keko[0].dir, MCX+keko[0].x, MCY+keko[0].y, -1);
	else sprite(18, MCX+keko[0].x, MCY+keko[0].y, -1);

}

/* Mueve a los fantasmas y los situa en pantalla */
void mover_fantasmas(void) {
	int cont;

	for (cont=1; cont<=n_fantasmas; cont++) {
		/* Borrado */
		sprite(0, MCX+keko[cont].x, MCY+keko[cont].y, -1);

		/* Movimiento */
		switch(keko[cont].dir) {
			case 1:	keko[cont].y-=keko[cont].vel; break;
			case 2:	keko[cont].y+=keko[cont].vel; break;
			case 3:	keko[cont].x-=keko[cont].vel; break;
			case 4:	keko[cont].x+=keko[cont].vel; break;
		}

		/* Dibujo */
		if (keko[cont].espera) /* Dormido */
			sprite(25, MCX+keko[cont].x, MCY+keko[cont].y - 1, cont+1);
		else if (keko[0].espera == 0) /* Estado Normal */
			if (!modo_agresivo) sprite(23, MCX+keko[cont].x, MCY+keko[cont].y, cont+1);
			else sprite(26, MCX+keko[cont].x, MCY+keko[cont].y, cont+1);
		else if (keko[0].espera > 54) /* Superpildora */
			sprite(24, MCX+keko[cont].x, MCY+keko[cont].y, cont+1);
			else /* Parpadeo fin de superpildora */
			sprite(23+(keko[0].espera/3)%2, MCX+keko[cont].x, MCY+keko[cont].y, cont+1);
	}
}

/* Controla a los fantasmas (IA) */
void control_fantasmas(void) {
	int cont, c2, caminos;
	for (cont=1; cont<=n_fantasmas; cont++) {

		caminos = 0;

		if (keko[cont].espera > 0) {
			keko[cont].espera--;
			keko[cont].dir = 0;
		} else {

			/* Si estaba en espera */
			if (keko[cont].dir == 0) {
				keko[cont].dir = 4;
			}

			/* Calculo de la velocidad */
			keko[cont].vel = velocidad();

			cuadra_keko(cont, keko[cont].vel); /* Ajuste a cuadricula */

			if ((keko[cont].x%30 == 0 && keko[cont].y%30 == 0)){
				/* Si encuentra otro camino */
				for (c2=1; c2<=4; c2++) if (!comprueba_camino(keko[cont],c2)) caminos++;
				if (caminos > 2) {
					camino_aleatorio(cont);
					if (modo_agresivo) ia_agresivo(cont);
					}
				/* Choque contra muros */
				if (comprueba_choque(keko[cont]) != 0) {
					keko[cont].dir=random(4)+1;
					while (comprueba_camino(keko[cont],keko[cont].dir))
						if (++keko[cont].dir>4) keko[cont].dir=1;
					if (modo_agresivo) ia_agresivo(cont);
				}
			}
		}
	/***************************************/
	/* Encuentro entre fantasmas y jugador */
	/***************************************/
	  if (keko[0].x<keko[cont].x+20 && keko[0].x>keko[cont].x-20 &&
		  keko[0].y<keko[cont].y+20 && keko[0].y>keko[cont].y-20) {
		  if (keko[0].espera > 0) {
			/* Fantasma Comido si no esta en espera */
			if (keko[cont].espera == 0) {
				delay(200);
				sprite(0,MCX+keko[cont].x,MCY+keko[cont].y, -1);
				keko[cont].dir = 0;
				keko[cont].x = 7*30;
				keko[cont].y = 6*30;
				keko[cont].espera = 36;
				puntos+=P_FANTASMA;
				dibuja_puntuacion();
				set_rotulo(keko[0].x+14,keko[0].y+8,"100",3);
			}
		  } else {
			/* Jugador Comido */
			vidas--;
			if (vidas >= 0) {
				delay(1000);
				flash(9);
				dibuja_mapa();
				dibuja_puntuacion();
				reset_nivel();
			}
		  }
	  }
	}
}

/* IA Agresiva de los fantasmas */
void ia_agresivo(int cont) {
	if (keko[0].y < keko[cont].y && !comprueba_camino(keko[cont], 1)
		&& keko[cont].dir != 2) keko[cont].dir = 1;
	if (keko[0].y > keko[cont].y && !comprueba_camino(keko[cont], 2)
		&& keko[cont].dir != 1)	keko[cont].dir = 2;
	if (keko[0].x < keko[cont].x && !comprueba_camino(keko[cont], 3)
		&& keko[cont].dir != 4)	keko[cont].dir = 3;
	if (keko[0].x > keko[cont].x && !comprueba_camino(keko[cont], 4)
		&& keko[cont].dir != 3)	keko[cont].dir = 4;
}

/* Controla al jugador */
void control_jugador() {
	/* Captura teclado */
	if (kbhit()) tecla = getch();
	/* Reduce las comprobaciones solamente cuando se encuentre
	   en una sola casilla, no en dos */
	if (keko[0].x%30 == 0 && keko[0].y%30 == 0) {
		switch(tecla) {
			case 75: keko[0].dir = 3; tecla = NULL; break;
			case 77: keko[0].dir = 4; tecla = NULL; break;
			case 72: keko[0].dir = 1; tecla = NULL; break;
			case 80: keko[0].dir = 2; tecla = NULL; break;
		}
		/* Quita las pildoras cuando las come */
		if (comprueba_casilla(keko[0]) == 'q' ||
			comprueba_casilla(keko[0]) == 'r') {
			if (comprueba_casilla(keko[0]) == 'r') {
				/* Superpildora */
				keko[0].espera=194-(nivel*10);
				puntos+=P_SPILDORA-P_PILDORA;
			}
			mapa[keko[0].y/30][keko[0].x/30] = ' ';
			pildoras++; puntos+=P_PILDORA;
			dibuja_puntuacion();
		}
		/* Para al jugador cuando choca */
		if (comprueba_choque(keko[0]) != 0) keko[0].dir = 0;
	}
}

/* Comprueba si un elemento ha chocado con un muro */
int comprueba_choque (S_KEKO p) {
	switch(p.dir) {
			case 1:	if (mapa[p.y/30 - 1][p.x/30] < 'q' &&
						mapa[p.y/30 - 1][p.x/30] > 'a') return 1; break;
			case 2:	if (mapa[p.y/30 + 1][p.x/30] < 'q' &&
						mapa[p.y/30 + 1][p.x/30] > 'a') return 2; break;
			case 3:	if (mapa[p.y/30][p.x/30 - 1] < 'q' &&
						mapa[p.y/30][p.x/30 - 1] > 'a') return 3; break;
			case 4:	if (mapa[p.y/30][p.x/30 + 1] < 'q' &&
						mapa[p.y/30][p.x/30 + 1] > 'a') return 4; break;
		}
	return 0;
}

/* Comprueba si un camino está libre */
int comprueba_camino (S_KEKO p, char dir) {
	switch(dir) {
		case 1:	if (mapa[p.y/30 - 1][p.x/30] < 'q' &&
					mapa[p.y/30 - 1][p.x/30] > 'a') return 1; break;
		case 2:	if (mapa[p.y/30 + 1][p.x/30] < 'q' &&
					mapa[p.y/30 + 1][p.x/30] > 'a') return 2; break;
		case 3:	if (mapa[p.y/30][p.x/30 - 1] < 'q' &&
					mapa[p.y/30][p.x/30 - 1] > 'a') return 3; break;
		case 4:	if (mapa[p.y/30][p.x/30 + 1] < 'q' &&
					mapa[p.y/30][p.x/30 + 1] > 'a') return 4; break;

	}
	return 0;
}

/* Comprueba el contenido de una casilla en el mapa */
int comprueba_casilla (S_KEKO p) {
	return mapa[p.y/30][p.x/30];
}

/* Asigna un camino aleatoriamente */
void camino_aleatorio(int cont) {
	int c2;
	for (c2=1; c2<=4; c2++)
		if (!comprueba_camino(keko[cont],c2) && !random(6))
			keko[cont].dir=c2;
}

/* Posiciona un elemento en una cuadricula */
void cuadra_keko (int num, float grid) {
	if (fmod(keko[num].x,grid) < grid/2) keko[num].x = ceil(keko[num].x/grid)*grid;
	else keko[num].x = floor(keko[num].x/grid)*grid;
	if (fmod(keko[num].y,grid) < grid/2) keko[num].y = ceil(keko[num].y/grid)*grid;
	else keko[num].y = floor(keko[num].y/grid)*grid;
}

/* Calcula el numero de píldoras existentes en un mapa */
int num_pild(void) {
	int cont=0, x, y;
	for (x=0; x<MTX; x++)
		for (y=0; y<MTY; y++)
			if (mapa[y][x] == 'q' || mapa[y][x] == 'r') cont++;
	return cont;
}

/* Devuelve la velocidad de los fantasmas */
int velocidad(void) {

	int vel;

	switch(nivel) {
		case 0: case 1: case 2: vel = 2; break;
		case 3: case 4: case 5: case 6: case 7: case 8: vel = 3; break;
		case 9: case 10: case 11: case 12: case 13: vel = 5; break;
		default: vel = 10;
	}

	if (keko[0].espera) vel/=2; /* Velocidad fantasmas Superpildora */
	if (vel <= 0) vel = 1;

	return vel;

}

/* Elimina todos los rotulos */
void reset_rotulos(void) {
	int cont;
	for (cont=0; cont<MAX_ROTULOS; cont++) rotulo[cont].espera=0;
}

/* Establece un rotulo en pantalla */
void set_rotulo(int x, int y, char texto[10], int tiempo) {
	int ok=0, cont=0;
	while (!ok && cont < MAX_ROTULOS) {
		if (rotulo[cont].espera < 0) {
			rotulo[cont].x = x;
			rotulo[cont].y = y;
			rotulo[cont].espera = tiempo*18;
			strcpy(rotulo[cont].texto,texto);
			ok = 1;
		}
		cont++;
	}
}

/* Dibuja los rotulos activos y controla su tiempo */
void dibuja_rotulos(void) {
	int cont;
	for (cont=0; cont<MAX_ROTULOS; cont++) {
		if (rotulo[cont].espera > -1) {
			if (rotulo[cont].espera > 0) setcolor(15);
			else setcolor(0);
			outtextxy(MCX+rotulo[cont].x,MCY+rotulo[cont].y,rotulo[cont].texto);
			rotulo[cont].espera--;
			}
	}
}

/* Dibuja el mapa en pantalla */
void dibuja_mapa(void) {
	char x, y;
	for (x=0; x<MTX; x++)
		for (y=0; y<MTY; y++)
			sprite(mapa[y][x]-'a', MCX+x*30, MCY+y*30, -1);
}

/* Dibuja solo las pildoras existentes del mapa en pantalla */
void dibuja_pild(void) {
	char x, y;
	for (x=0; x<MTX; x++)
		for (y=0; y<MTY; y++) {
			if (mapa[y][x] == 'q') sprite(16, MCX+x*30, MCY+y*30, -1);
			if (mapa[y][x] == 'r') sprite(17, MCX+x*30, MCY+y*30, -1);
			}
}

/* Dibuja la barra de puntuacion */
void dibuja_puntuacion(void) {
	char t_puntuacion[70], tmp[5];
	setfillstyle(1,0);
	setcolor(15);
	bar(MCX,MCY+MTY*30,MCX+MTX*30,MCY+MTY*30+16);
	strcpy(t_puntuacion, "NIVEL: ");
	itoa(nivel, tmp, 10);
	strcat(t_puntuacion, tmp);
	strcat(t_puntuacion, "   PUNTOS: ");
	itoa(puntos, tmp, 10);
	strcat(t_puntuacion, tmp);
	strcat(t_puntuacion, "   VIDAS: ");
	itoa(vidas, tmp, 10);
	strcat(t_puntuacion, tmp);
	strcat(t_puntuacion, "   TIEMPO: ");
	itoa(tiempo, tmp, 10);
	strcat(t_puntuacion, tmp);
	outtextxy(MCX+MTX*15,MCY+MTY*30, t_puntuacion);
}

/* Hace un Flash en pantalla de un color determinado */
void flash(char color) {
	setfillstyle(SOLID_FILL,color);
	bar(0,0,649,479);
	delay(60);
	cleardevice();
}

/* Partida ganada */
void partida_ganada(void) {
	delay(1000);
	flash(15);
	pasa_nivel();
	carga_mapa(nivel);
	dibuja_mapa();
	dibuja_puntuacion();
}

/* Partida perdida */
void partida_perdida(void) {
	delay(1000);
	flash(12);
	setcolor(4);
	outtextxy(320,240, "GAME OVER");
	getch();
	cleardevice();
	carga_mapa(nivel);
	dibuja_mapa();
	pausa_inicio();
	inicializar_juego();
	dibuja_puntuacion();
}

/* Dibuja un gráfico */
void sprite(char id, int x, int y, char color) {

	/* Cuadros de 30x30 pixels */

	switch(id) {
		/* Paredes */

/*   */	case 0: setfillstyle(0,0); bar(x, y, x+29, y+29); break;
/* O */	case 1: setcolor(9); circle(x+14,y+14,14); arc(x+14,y+14,120,180,11);
				break;
/* ─ */	case 2: setcolor(9); line(x,y,x+29,y); line(x,y+28,x+29,y+28); break;
/* │ */	case 3: setcolor(9); line(x,y,x,y+29); line(x+28,y,x+28,y+29); break;
/*[─ */ case 4: setcolor(9); arc(x+14,y+14,90,270,14); line(x+14,y,x+29,y);
				line(x+14,y+28,x+29,y+28); arc(x+14,y+14,120,180,11); break;
/* ─]*/	case 5: setcolor(9); arc(x+14,y+14,-90,90,14); line(x+14,y,x,y);
				line(x+14,y+28,x,y+28); break;
/* | */	case 6: setcolor(9); arc(x+14,y+14,0,180,14); line(x,y+14,x,y+29);
				line(x+28,y+14,x+28,y+29); arc(x+14,y+14,120,180,11); break;
/* | */	case 7: setcolor(9); arc(x+14,y+14,180,360,14); line(x,y+14,x,y);
				line(x+28,y+14,x+28,y); break;
/* ┌ */	case 8: setcolor(9); arc(x+14,y+14,90,180,14); line(x,y+14,x,y+29);
				line(x+14,y,x+29,y); arc(x+14,y+14,120,180,11); break;
/* ┘ */	case 9: setcolor(9); arc(x+14,y+14,270,360,14); line(x,y+28,x+14,y+28);
				line(x+28,y,x+28,y+14); break;
/* ┐ */	case 10:setcolor(9); arc(x+14,y+14,0,90,14); line(x,y,x+14,y);
				line(x+28,y+14,x+28,y+29); break;
/* └ */	case 11:setcolor(9); arc(x+14,y+14,180,270,14); line(x,y,x,y+14);
				line(x+29,y+28,x+14,y+28); break;
/* ┴ */ case 12:setcolor(9); line(x,y+28,x+29,y+28); break;
/* ┬ */ case 13:setcolor(9); line(x,y,x+29,y); break;
/* ├ */ case 14:setcolor(9); line(x,y,x,y+29); break;
/* ┤ */ case 15:setcolor(9); line(x+28,y,x+28,y+29); break;

		/* Elementos */

/* · */	case 16:setcolor(15); circle(x+14,y+14,2); break;
/* o */	case 17:setcolor(15); circle(x+14,y+14,4); arc(x+14,y+14,90,180,3);
				break;
/* O */	case 18:setcolor(14); circle(x+14,y+14,14); break;
/* U */	case 19:setcolor(14); arc(x+14,y+14, 130, 40, 14);
				line(x+14,y+14,x+24,y+4); line(x+14,y+14,x+4,y+4);break;
/* D */	case 20:setcolor(14); arc(x+14,y+14, 320, 220, 14);
				line(x+14,y+14,x+24,y+24); line(x+14,y+14,x+4,y+24);break;
/* L */	case 21:setcolor(14); arc(x+14,y+14, 230, 130, 14);
				line(x+14,y+14,x+4,y+4); line(x+14,y+14,x+4,y+24);break;
/* R */	case 22:setcolor(14); arc(x+14,y+14, 50, 320, 14);
				line(x+14,y+14,x+24,y+4); line(x+14,y+14,x+24,y+24);break;
/* F */	case 23:setcolor(color); arc(x+14,y+14,0,180,14); line(x,y+14,x,y+28);
				line(x+28,y+14,x+28,y+28);
				line(x,y+28,x+4,y+24); line(x+4,y+24,x+9,y+28);
				line(x+9,y+28,x+14,y+24); line(x+14,y+24,x+19,y+28);
				line(x+19,y+28,x+23,y+24); line(x+23,y+24,x+28,y+28);
				line(x+7,y+9,x+11,y+9); line(x+21,y+9,x+17,y+9);
				arc(x+9,y+9,0,180,3); arc(x+19,y+9,0,180,3); break;
/*   */	case 24:setcolor(13); arc(x+14,y+14,0,180,14); line(x,y+14,x,y+28);
				line(x+28,y+14,x+28,y+28);
				line(x,y+28,x+4,y+24); line(x+4,y+24,x+9,y+28);
				line(x+9,y+28,x+14,y+24); line(x+14,y+24,x+19,y+28);
				line(x+19,y+28,x+23,y+24); line(x+23,y+24,x+28,y+28);
				circle(x+8,y+9,3); circle(x+20,y+9,3); break;
/* D */	case 25:setcolor(color); arc(x+14,y+14,0,180,14); line(x,y+14,x,y+28);
				line(x+28,y+14,x+28,y+28);
				line(x,y+28,x+4,y+24); line(x+4,y+24,x+9,y+28);
				line(x+9,y+28,x+14,y+24); line(x+14,y+24,x+19,y+28);
				line(x+19,y+28,x+23,y+24); line(x+23,y+24,x+28,y+28);
				line(x+7,y+9,x+11,y+9); line(x+21,y+9,x+17,y+9);
				break;
/* A */	case 26:setcolor(color); arc(x+14,y+14,0,180,14); line(x,y+14,x,y+28);
				line(x+28,y+14,x+28,y+28);
				line(x,y+28,x+4,y+24); line(x+4,y+24,x+9,y+28);
				line(x+9,y+28,x+14,y+24); line(x+14,y+24,x+19,y+28);
				line(x+19,y+28,x+23,y+24); line(x+23,y+24,x+28,y+28);
				rectangle(x+5,y+18,x+9,y+20); rectangle(x+9,y+18,x+14,y+20);
				rectangle(x+14,y+18,x+19,y+20); rectangle(x+19,y+18,x+23,y+20);
				arc(x+9,y+8,180,0,3); arc(x+19,y+8,180,0,3);
				line(x+7,y+8,x+21,y+8); break;
	}
}

/* Carga un mapa en memoria */
void carga_mapa(int mapanivel) {

	switch (mapanivel%5) {
	/* Define el mapa */

		default:

		strcpy(mapa[0],  "iccccccncccccck");
		strcpy(mapa[1],  "dqqqqqqdqqqqqqd");
		strcpy(mapa[2],  "dqicfqe fqeckqd");
		strcpy(mapa[3],  "dqhqqqqhqqqqhqd");
		strcpy(mapa[4],  "dqqqbqqrqqbqqqd");
		strcpy(mapa[5],  "dqgqqqickqqqgqd");
		strcpy(mapa[6],  "dqofqqd dqqepqd");
		strcpy(mapa[7],  "dqhqqej lfqqhqd");
		strcpy(mapa[8],  "dqqqqq   qqqqqd");
		strcpy(mapa[9],  "drifqgqbqgqekrd");
		strcpy(mapa[10], "dqhqqhq qhqqhqd");
		strcpy(mapa[11], "dqqqqqq qqqqqqd");
		strcpy(mapa[12], "lcccccccccccccj");
		break;

		case 4:

		strcpy(mapa[0],  "iccnccncnccccck");
		strcpy(mapa[1],  "dqrdqqhqhqqqqrd");
		strcpy(mapa[2],  "dqemfqqqqqicccp");
		strcpy(mapa[3],  "dqqqqqgqbqdqqqd");
		strcpy(mapa[4],  "dqicfqdqqqlckqd");
		strcpy(mapa[5],  "dqdqqqockqqrdqd");
		strcpy(mapa[6],  "dqlcccj dqgqdqd");
		strcpy(mapa[7],  "dqqqq   dqhqhqd");
		strcpy(mapa[8],  "dqicccf hqqqqqd");
		strcpy(mapa[9],  "dqdqqqqqqqgrgqd");
		strcpy(mapa[10], "dqlfqeccccmcjqd");
		strcpy(mapa[11], "drqqqqqqqqqqqqd");
		strcpy(mapa[12], "lcccccccccccccj");
		break;

		case 3:

		strcpy(mapa[0],  "icccncccccnccck");
		strcpy(mapa[1],  "dqqqhqqqqqhqqqd");
		strcpy(mapa[2],  "dqrqqqickqqqrqd");
		strcpy(mapa[3],  "dqqqgqhqhqgqqqd");
		strcpy(mapa[4],  "ofqej     lfqep");
		strcpy(mapa[5],  "dqqqq ecf qqqqd");
		strcpy(mapa[6],  "dqecf     ecfqd");
		strcpy(mapa[7],  "dqqqq ecf qqqqd");
		strcpy(mapa[8],  "ofqek     ifqep");
		strcpy(mapa[9],  "dqqqhqgqgqhqqqd");
		strcpy(mapa[10], "dqrqqqlcjqqqrqd");
		strcpy(mapa[11], "dqqqgq   qgqqqd");
		strcpy(mapa[12], "lcccmcccccmcccj");
		break;


		case 2:

		strcpy(mapa[0],  "iccccccccccccck");
		strcpy(mapa[1],  "dqqqqqqqqqqqqqd");
		strcpy(mapa[2],  "dqb b brb b bqd");
		strcpy(mapa[3],  "dqqqqqqqqqqqqqd");
		strcpy(mapa[4],  "dqb b     b bqd");
		strcpy(mapa[5],  "dqqqq ick qqqqd");
		strcpy(mapa[6],  "drb b d d b brd");
		strcpy(mapa[7],  "dqqqq h h qqqqd");
		strcpy(mapa[8],  "dqb b     b bqd");
		strcpy(mapa[9],  "dqqqqqqqqqqqqqd");
		strcpy(mapa[10], "dqb b b b b bqd");
		strcpy(mapa[11], "dqqqqq   qqqqqd");
		strcpy(mapa[12], "lcccccccccccccj");
		break;

		case 1:

		strcpy(mapa[0],  "icccncccccnccck");
		strcpy(mapa[1],  "dqqqdqqrqqdqqqd");
		strcpy(mapa[2],  "dqgqdqqqqqdqgqd");
		strcpy(mapa[3],  "dqhqhqqqqqhqhqd");
		strcpy(mapa[4],  "dq q       q qd");
		strcpy(mapa[5],  "dqgqg ecf gqgqd");
		strcpy(mapa[6],  "dqdqd     dqdqd");
		strcpy(mapa[7],  "dqdqd g g dqdqd");
		strcpy(mapa[8],  "dqdqlcj lcjqdqd");
		strcpy(mapa[9],  "dqdqqqqqqqqqdqd");
		strcpy(mapa[10], "dqlcccf ecccjqd");
		strcpy(mapa[11], "drqqqq   qqqqrd");
		strcpy(mapa[12], "lcccccccccccccj");
		break;


	}

	pild_total = num_pild(); 	/* Calcula las pildoras totales */
}
