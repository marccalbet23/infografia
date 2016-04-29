#include "application.h"
#include "utils.h"
#include "image.h"

Image *img = NULL;
Image *img_aux = NULL;
int mode, mod; // modo no pintar = 0 / modo pintar = 1; / modo borrar = 2;
int type = 0; // type 0 -> pintar,borrar  type 1 -> mandelbrot
Color pintar; // color con el que pintamos
Color fondo, fondo_aux; // fondo -> fondo actual / fondo_aux -> fondo antiguo
int deltax;
float x1, x0;
float deltay, yy1, yy0;
int punt = 0;
int size;
int clickrecta = 0;
int netejar = 0;
bool noclick;
double cRe, cIm;
double zoom , moveX , moveY ;
Color color; 
int maxIterations ; // iteraciones max que hara el algoritmo

double time2, oldTime, frameTime; //current and old time, and their difference (for input)



Application::Application(const char* caption, int width, int height)
{
	this->window = createWindow(caption, width, height);

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	this->window_width = w;
	this->window_height = h;
	this->keystate = SDL_GetKeyboardState(NULL);
}

//Here we have already GL working, so we can create meshes and textures
void Application::init(void)
{
	std::cout << "initiating app..." << std::endl;
	std::cout << "Pintar: 1->Negro / 2->Blanco / 3->Azul / 4->Cian / 5->Gris / 6->Verde / 7->Lila / 8->Rojo / 9->Amarillo" << std::endl;
	std::cout << "Fondo: a->Negro / s->Blanco / d->Azul / f->Cian / g->Gris / h->Verde / j->Lila / k->Rojo / l->Amarillo" << std::endl;
	img = new Image(window_width, window_height);
	//here add your init stuff
	mode = 0;
	pintar = Color::WHITE;
	fondo = fondo_aux = Color::BLACK;
	noclick = true;

	size = 4;
	maxIterations = 128;
	zoom = 1, moveX = 0, moveY = 0;
	cRe = -0.7;
	cIm = 0.27015;
}

void drawPoint(Image* img, int x, int y, Color c) {

	for (int i = x - size; i <= x + size; ++i) {
		for (int j = y - size; j <= y + size; ++j) {
			img->setPixelSafe(i, j, c);
		}
	}
}

void clear(Image* img, int x, int y, Color c) {

	for (int i = 0; i <= x; ++i) {
		for (int j = 0; j <= y; ++j) {
			img->setPixelSafe(i, j, c);
		}
	}
}



void Line(float x1, float y1, float x2, float y2, Color c) {

	const bool steep = (fabs(y2 - y1) > fabs(x2 - x1));
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2)
	{
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	const float dx = x2 - x1;
	const float dy = fabs(y2 - y1);

	float error = dx / 2.0f;
	const int ystep = (y1 < y2) ? 1 : -1;
	int y = (int)y1;

	const int maxX = (int)x2;

	for (int x = (int)x1; x < maxX; x++)
	{
		if (steep)
		{
			//img->setPixel(y, x, c);
			if (mode == 2) {
				drawPoint(img, y, x, fondo);//drawPoint(img, y, x, fondo);
			}
			else
				drawPoint(img, y, x, pintar); //drawPoint(img, y, x, pintar);
		}
		else
		{
			//img->setPixel(x, y, c);
			if (mode == 2)
				drawPoint(img, x, y, fondo); //drawPoint(img, x, y, fondo);
			else
				drawPoint(img, x, y, pintar); //drawPoint(img, x, y, pintar);
		}

		error -= dy;
		if (error < 0)
		{
			y += ystep;
			error += dx;
		}
	}
}


//render one frame
void Application::render(void)
{
	//Create a new Image (we could reuse the same one to be faster)
	//img = new Image( window_width, window_height);

	//fill the image with some random data

	if (type == 0) {
		if (netejar == 1) {			
			clear(img, window_width, window_height, Color::BLACK);
			netejar = 0;
			mode = 1;
			
		}
		else if ((mode == 1) && (mod != 3)) { // modo pintar
			if (noclick) drawPoint(img, mouse_position.x, window_height - mouse_position.y, pintar);
			else Line(x0, yy0, mouse_position.x, window_height - mouse_position.y, pintar);
			noclick = false;
			x0 = mouse_position.x;
			yy0 = window_height - mouse_position.y;
		}
		else if (mode == 2) { //modo borrar
			//size = 5;
			//Line(x0, yy0, mouse_position.x, window_height - mouse_position.y, fondo);
			if (noclick) drawPoint(img, mouse_position.x, window_height - mouse_position.y, fondo);
			else Line(x0, yy0, mouse_position.x, window_height - mouse_position.y, fondo);
			noclick = false;
			x0 = mouse_position.x;
			yy0 = window_height - mouse_position.y;
		}
		else if (mod == 3) { // modo rectes
			noclick = false;
			if (clickrecta == 2) {
				Line(x0, yy0, x1, yy1, pintar);
				clickrecta = 0;
			}
		}
		else noclick = true;
	}

	else
	{

		double newRe, newIm, oldRe, oldIm;

		for (int y = 0; y < window_height; y++)
			for (int x = 0; x < window_width; x++)
			{

				newRe = 1.5 * (x - window_height / 2) / (0.5 * zoom * window_width) + moveX;
				newIm = (y - window_height / 2) / (0.5 * zoom * window_height) + moveY;

				int i;

				for (i = 0; i < maxIterations; i++)
				{

					oldRe = newRe;
					oldIm = newIm;

					newRe = oldRe * oldRe - oldIm * oldIm + cRe;
					newIm = 2 * oldRe * oldIm + cIm;

					if ((newRe * newRe + newIm * newIm) > 4) break;
				}

				color = (Color(i % 256, 255, 255 * (i < maxIterations)));

				img->setPixel(x, y, color);
			}
		oldTime = time2;
		this->time = time;
		frameTime = time - oldTime;

		netejar = 1;

	}

	

	//drawPoint(img, x, y, Color::WHITE);

	/*for(unsigned int x = 0; x < img.width; x++)
	for(unsigned int y = 0; y < img.height; y++)
	{
	img.setPixel(x,y, Color( rand(), rand(), rand() ) );
	}
	*/
	//send image to screen
	showImage(img);
}

//called after render
void Application::update(double seconds_elapsed)
{
	if (keystate[SDL_SCANCODE_SPACE])
	{
		img_aux = new Image(window_width, window_height);
		for (int i = 0; i <= window_width; ++i) {
			for (int j = 0; j <= window_height; ++j) {
				if (img->getPixelSafe(i, j).r == fondo_aux.r && img->getPixelSafe(i, j).b == fondo_aux.b && img->getPixelSafe(i, j).g == fondo_aux.g)
					img_aux->setPixelSafe(i, j, fondo);
				else img_aux->setPixelSafe(i, j, (img->getPixelSafe(i, j)));
			}
		}
		img = img_aux;
	}
}

//keyboard press event 
void Application::onKeyPressed(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_ESCAPE: exit(0); break; //ESC key, kill the app
	//cambiar color fondo		//1->Negro / 2->Blanco / 3->Azul / 4->Cian / 5->Gris / 6->Verde / 7->Lila / 8->Rojo / 9->Amarillo
	case SDLK_1: pintar = Color::BLACK; break;
	case SDLK_2: pintar = Color::WHITE; break;
	case SDLK_3: pintar = Color::BLUE; break;
	case SDLK_4: pintar = Color::CYAN; break;
	case SDLK_5: pintar = Color::GRAY; break;
	case SDLK_6: pintar = Color::GREEN; break;
	case SDLK_7: pintar = Color::PURPLE; break;
	case SDLK_8: pintar = Color::RED; break;
	case SDLK_9: pintar = Color::YELLOW; break;
		//cambiar color fondo
	case SDLK_a: fondo_aux = fondo; fondo = Color::BLACK; break;
	case SDLK_s: fondo_aux = fondo; fondo = Color::WHITE; break;
	case SDLK_d: fondo_aux = fondo; fondo = Color::BLUE; break;
	case SDLK_f: fondo_aux = fondo; fondo = Color::CYAN; break;
	case SDLK_g: fondo_aux = fondo; fondo = Color::GRAY; break;
	case SDLK_h: fondo_aux = fondo; fondo = Color::GREEN; break;
	case SDLK_j: fondo_aux = fondo; fondo = Color::PURPLE; break;
	case SDLK_k: fondo_aux = fondo; fondo = Color::RED; break;
	case SDLK_l: fondo_aux = fondo; fondo = Color::YELLOW; break;
		// modo recta
	case SDLK_r: mod = 3; break;
		// modo pintar
	case SDLK_p: mod = 0; break;
		// alternar figura/ pintar
	case SDLK_m: if (type == 0) type = 1; else type = 0;  break;
	case SDLK_MINUS: if (size >= 2) size = size - 1; break;
	case SDLK_KP_MINUS:if (size >= 2) size = size - 1;
		zoom /= pow(1.001, frameTime); break;
	case SDLK_KP_PLUS: size = size + 1;
		zoom *= pow(1.001, frameTime); break;
		//mover figura
	case SDLK_DOWN:  moveY += 0.0003 * frameTime / zoom; break;
	case SDLK_UP: moveY -= 0.0003 * frameTime / zoom; break;
	case SDLK_RIGHT: moveX -= 0.0003 * frameTime / zoom; break;
	case SDLK_LEFT: moveX += 0.0003 * frameTime / zoom; break;
		//cambiar forma figura
	case SDLK_KP_2: cIm += 0.0002 * frameTime / zoom; break;
	case SDLK_KP_8: cIm -= 0.0002 * frameTime / zoom; break;
	case SDLK_KP_6: cRe += 0.0002 * frameTime / zoom; break;
	case SDLK_KP_4: cRe -= 0.0002 * frameTime / zoom; break;
		//variar numero de iteraciones
	case SDLK_KP_MULTIPLY: maxIterations *= 2; break;
	case SDLK_KP_DIVIDE: if (maxIterations > 2) maxIterations /= 2; break;

	}
}

//mouse button event
void Application::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse pressed
	{
		//modo pintar
		mode = 1;

		if (mod == 3) {
			if (clickrecta == 0) {
				x0 = mouse_position.x;
				yy0 = window_height - mouse_position.y;
				clickrecta = 1;
			}
			else {
				x1 = mouse_position.x;
				yy1 = window_height - mouse_position.y;
				clickrecta = 2;
			}
		}




	}
	else if (event.button == SDL_BUTTON_RIGHT) //right mouse pressed
	{
		//modo borrar
		mode = 2;


	}
}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_LEFT) //left mouse unpressed
	{
		//modo no pintar
		mode = 0;
		/*if (mod == 3) {
			x1 = mouse_position.x;
			yy1 = mouse_position.y;
		}*/

	}
	else if (event.button == SDL_BUTTON_RIGHT) //right mouse unpressed
	{
		//modo pintar
		mode = 0;

	}
}

//when the app starts
void Application::start()
{
	std::cout << "launching loop..." << std::endl;
	launchLoop(this);
}