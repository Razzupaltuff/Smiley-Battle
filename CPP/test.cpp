#include "vector.h"
#include "matrix.h"
#include "icosphere.h"
#include "sdl.h"

int main(int argc, char* argv[]) {
	SDL_SetMainReady();
	SDL_Init(SDL_INIT_VIDEO);
	CRectangleIcoSphere s;
	CList<CString>	n;
	s.Create(4);
	SDL_Quit();
	return 0;
}