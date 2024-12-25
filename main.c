#include <SDL.h>
#include <stdio.h>
#include <SDL_image.h>
#include "asset.xpm"

// variables
const int block = 24;
const int scaledBlock = block * 3;
const int width = scaledBlock * 3;
const int height = width;

SDL_Window* window;
SDL_Renderer* renderer;

SDL_Texture* assets_texture;

const SDL_Rect grid = {0, block, width, height};
const SDL_Rect gridOut = {0, 0, width, height};
const SDL_Rect cross = {0, 0, block, block};
const SDL_Rect circle = {block, 0, block, block};

typedef enum Owner {
	N,
	X,
	O
} Owner;

typedef struct Square {
	Owner owner;
	SDL_Rect pos;
} Square;

Square squares[9];
Owner winner;
Owner turn;
int moveCounter;

int init() {
	// initialization
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Cannot initialize SDL: %s\n", SDL_GetError());
		return 0;
	}

	SDL_Surface* assets;
	assets = IMG_ReadXPMFromArray(asset_xpm);

	SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_SHOWN, &window, &renderer);
	if (!window) {
		fprintf(stderr, "Cannot create window: %s\n", SDL_GetError());
		goto windowCreationFail;
	}

	assets_texture = SDL_CreateTextureFromSurface(renderer, assets);
	SDL_FreeSurface(assets);

	return 1;
	
windowCreationFail:
	SDL_FreeSurface(assets);
	SDL_Quit();

	return 0;
}

void deinit() {
	SDL_DestroyTexture(assets_texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
}

void render(int line, SDL_Point* start, SDL_Point* finish) {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, assets_texture, &grid, &gridOut);

	for (int i = 0; i < 9; i++) {
		switch (squares[i].owner) {
			case N: break;
			case X: SDL_RenderCopy(renderer, assets_texture, &cross, &squares[i].pos); break;
			case O: SDL_RenderCopy(renderer, assets_texture, &circle, &squares[i].pos); break;
		}
	}

	if (line) {
		if (winner == X)
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		else
			SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

		SDL_RenderDrawLine(renderer, start->x, start->y, finish->x, finish->y);
	}

	SDL_RenderPresent(renderer);
}


char title_buf[20];
void titlemaker(Owner winner, int draw) {
	if (winner == N)
		sprintf(title_buf, "SDLTTT - %c's turn", turn == X ? 'X' : 'O');
	else
		sprintf(title_buf, "SDLTTT - %c won!", winner == X ? 'X' : 'O');

	if (draw) 
		sprintf(title_buf, "SDLTTT - Draw!");

	SDL_SetWindowTitle(window, title_buf);
}

void checkWinner() {
	Owner player = X;
	SDL_Point start = {0, 0,}, finish = {216, 216};
	int count;

again:
	count = 0;
	// horizontal
	for (int i = 0; i < 3; i++)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) {
		winner = player;
		start.y = 36;
		finish.y = start.y;
	}

	count = 0;
	for (int i = 3; i < 6; i++)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) {
		winner = player;
		start.y = 108;
		finish.y = start.y;
	}

	count = 0;
	for (int i = 6; i < 9; i++)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) {
		winner = player;
		start.y = 180;
		finish.y = start.y;
	}

	// vertical
	count = 0;
	for (int i = 0; i < 7; i += 3)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) {
		winner = player;
		start.x = 36;
		finish.x = start.x;
	}

	count = 0;
	for (int i = 1; i < 8; i += 3)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) {
		winner = player;
		start.x = 108;
		finish.x = start.x;
	}

	count = 0;
	for (int i = 2; i < 9; i += 3)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) {
		winner = player;
		start.x = 180;
		finish.x = start.x;
	}

	// diagonal
	count = 0;
	for (int i = 0; i < 9; i += 4)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) winner = player;

	count = 0;
	for (int i = 2; i < 7; i += 2)
		count += squares[i].owner == player ? 1 : 0;
	if (count == 3) {
		winner = player;
		start.x = 216;
		finish.x = 0;
	}

	if (winner == N) {
		if (player == X) {
			player = O;
			goto again;
		}
		render(0, 0, 0);
	} else {
		render (1, &start, &finish);
	}

	titlemaker(winner, 0);
}
void clearBoard() {
	int squareY = scaledBlock * -1;
	int squareX = 0;
	for (int i = 0; i < 9; i++) {
		if (i % 3 == 0) {
			squareY += scaledBlock;
			squareX = 0;
		}
		squares[i].owner = N;
		squares[i].pos.x = squareX;
		squares[i].pos.y = squareY;
		squares[i].pos.w = scaledBlock;
		squares[i].pos.h = scaledBlock;
		squareX += scaledBlock;
	}
	turn = X;
	winner = N;
	moveCounter = 0;
}

int main(int argc, char* argv[]) {
	if (!init()) return 1;
	clearBoard();

	int running = 1;
	SDL_Event e;
	SDL_Point mousepos;

	render(0, 0, 0);
	titlemaker(N, 0);

	Uint64 start, end;
	while (running) {
		start = SDL_GetTicks64();

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT: running = 0; break;
				case SDL_MOUSEBUTTONDOWN:
					if (SDL_BUTTON(SDL_GetMouseState(&mousepos.x, &mousepos.y)) == 1) {
						for (int i = 0; i < 9; i++) {
							if (winner != N || moveCounter == 9) {
								clearBoard();
								render(0, 0, 0);
								titlemaker(N, 0);
								break;
							}
							if (SDL_PointInRect(&mousepos, &squares[i].pos) && squares[i].owner == N) {
								squares[i].owner = turn;
								if (turn == X) turn = O;
								else turn = X;
								checkWinner();
								moveCounter++;

								if (moveCounter == 9 && winner == N)
									titlemaker(N, 1);
								break;
							}
						}
					}
					break;
				case SDL_KEYDOWN:
					if (e.key.keysym.sym == SDLK_ESCAPE)
						running = 0;
					break;
			}
		}

		end = SDL_GetTicks64();
		if ((1000 / 30) > (end - start)) // poll 30fps
			SDL_Delay((1000 / 30) - (end - start));
	}

	// deinitialization
	deinit();
	return 0;
}
