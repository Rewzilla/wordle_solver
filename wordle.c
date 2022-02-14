
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

struct word {
	int	score;
	char word[6];
};

struct wordlist {
	struct word *words;
	int size;
};

void prune_words(struct wordlist *wl, char guess[6], char response[6]) {

	int i, j, k, l, s;

	for (i=0; i<5; i++) {

		printf("\rPruning words: %.02f%%", (( (float)i * 100) / 6) );

		switch (response[i]) {

			case 'b':
			case 'B':
				for (j=0; j<wl->size; j++) {

					if (wl->words[j].score == -1)
						continue;

					for (k=0; k<5; k++) {
						if (wl->words[j].word[k] == guess[i]) {
							wl->words[j].score = -1;
							break;
						}
					}

				}
				break;

			case 'y':
			case 'Y':
				for (j=0; j<wl->size; j++) {

					if (wl->words[j].score == -1)
						continue;

					if (wl->words[j].word[i] == guess[i]) {
						wl->words[j].score = -1;
						continue;
					}

					l = 0;

					for (k=0; k<5; k++)
						if (wl->words[j].word[k] == guess[i]) {
							l = 1;
							break;
						}

					if (l == 0) {
						wl->words[j].score = -1;
					}

				}
				break;

			case 'g':
			case 'G':
				for (j=0; j<wl->size; j++) {

					if (wl->words[j].score == -1)
						continue;

					if (wl->words[j].word[i] != guess[i]) {
						wl->words[j].score = -1;
					}

				}
				break;

			default:
				printf("\rError, '%c' is invalid, must be either B(lack), Y(ellow), or G(reen)\n", response[i]);
				exit(0);

		}

	}

	printf("\rPruning words: DONE!   \n");

	s = wl->size;

	for (i=0; i<wl->size; ) {

		printf("\rDefraggin wordlist: %.02f%%", (( (float)i * 100) / wl->size) );

		if (wl->words[i].score == -1) {
			for (j=i; j<wl->size; j++) {
				memcpy(&wl->words[j], &wl->words[j+1], sizeof(struct word));
			}
			wl->size--;
		} else {
			i++;
		}

	}

	printf("\rDefragging wordlist: DONE!   \n");
	printf("Removed a total of %d words, %d remaining\n", (s - wl->size), wl->size);

}

void sort_words(struct wordlist *wl) {

	int i, j;
	struct word tmp;

	for (i=0; i<(wl->size - 1); i++) {

		printf("\rSorting words: %.02f%%", (( (float)i * 100) / wl->size) );

		for (j=0; j<(wl->size - i - 1); j++) {

			if (wl->words[j].score < wl->words[j+1].score) {
				memcpy(&tmp, &wl->words[j], sizeof(struct word));
				memcpy(&wl->words[j], &wl->words[j+1], sizeof(struct word));
				memcpy(&wl->words[j+1], &tmp, sizeof(struct word));
			}

		}

	}

	printf("\rSorting words: DONE!   \n");

}

void rescore_words(struct wordlist *wl) {

	int i, j, k, l;
	float progress;
	char seen[5];

	#pragma omp parallel for private(i,j,k,l,seen)
	for (i=0; i<wl->size; i++) {

		printf("\rRescoring words: %.02f%%", (( (float)i * 100) / wl->size) );

		wl->words[i].score = 0;

		for (j=0; j<wl->size; j++) {

			memset(seen, '\0', 5);

			for (k=0; k<5; k++) {

				for (l=0; l<5; l++)
					if (wl->words[i].word[k] == seen[l])
						goto skip_letter;

				seen[k] = wl->words[i].word[k];

				if (wl->words[i].word[k] == wl->words[j].word[k])
					wl->words[i].score++;

				for (l=0; l<5; l++)
					if (wl->words[i].word[k] == wl->words[j].word[l])
						wl->words[i].score++;

			}

			skip_letter:

		}

	}

	printf("\rRescoring words: DONE!   \n");

}

void print_summary(struct wordlist *wl, int amount) {

	int i;

	printf("Word list summary (size=%d):\n", wl->size);
	printf("%8s | %5s | %5s\n", "Index", "Score", "Word");

	for (i=0; i<amount; i++) {

		printf("%8d | %5d | %5s\n", i, wl->words[i].score, wl->words[i].word);

	}

	printf("\n ... %d other words ... \n\n", (wl->size - (amount * 2)));

	for (i=(wl->size - amount); i<wl->size; i++) {

		printf("%8d | %5d | %5s\n", i, wl->words[i].score, wl->words[i].word);

	}

}

void print_words(struct wordlist *wl) {

	int i;

	printf("Word list (size=%d):\n", wl->size);
	printf("%8s | %5s | %5s\n", "Index", "Score", "Word");

	for (i=0; i<wl->size; i++) {

		printf("%8d | %5d | %5s\n", i, wl->words[i].score, wl->words[i].word);

	}

}

void load_words(struct wordlist *wl, char *filename) {

	FILE *fp;
	int len;
	char tmp[5];

	wl->words = NULL;
	wl->size = 0;

	fp = fopen(filename, "r");

	if (!fp) {
		printf("Error reading word list from file '%s'\n", filename);
		exit(0);
	}

	while (fscanf(fp, "%5s ", tmp) == 1) {

		wl->words = realloc(wl->words, sizeof(struct word) * (wl->size + 1));

		memset(wl->words[wl->size].word, '\0', 6);
		wl->words[wl->size].score = 0;

		if (!wl->words) {
			printf("Error allocating memory for word list\n");
			exit(0);
		}

		memcpy(wl->words[wl->size].word, tmp, 5);

		wl->size++;

	}

	printf("Loaded %d words from '%s'\n", wl->size, filename);

}

int main(int argc, char *argv[]) {

	struct wordlist *wl;
	int guess;
	char response[6], known[5];

	if (argc != 2) {

		printf("Usage: %s <words.txt>\n", argv[0]);
		printf("  <words.txt> should be a plaintext file containing all words of length 5, newline-seperated.\n");

		exit(0);

	}

	wl = malloc(sizeof(struct wordlist));

	load_words(wl, argv[1]);
	rescore_words(wl);
	sort_words(wl);
	print_summary(wl, 10);


	/*
	srand(time(0));
	int r = 931; //rand() % wl->size;
	char word[6];
	strncpy(word, wl->words[r].word, 6);
	printf("Random word %d : %s\n", r, word);
	*/

	for (guess=1; guess<=6; guess++) {

		printf("GUESS #%d: %s\n", guess, wl->words[0].word);

		memset(response, '\0', 6);
		printf("RESPONSE: ");
		scanf(" %5s", response);

		if (strcmp(response, "ggggg") == 0 || strcmp(response, "GGGGG") == 0) {
			printf("Got it in %d guesses!  The word is '%s'\n", guess, wl->words[0].word);
			exit(0);
		}

		prune_words(wl, wl->words[0].word, response);
		rescore_words(wl);
		sort_words(wl);

		if (wl->size > 20)
			print_summary(wl, 10);
		else
			print_words(wl);

	}

}