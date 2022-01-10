# wordle_solver
A C program to solve Wordle puzzles (https://www.powerlanguage.co.uk/wordle/)

# Building
```
make
```

# Usage
```
./wordle <dict.txt>
```

A dictionary of only 5-letter words is included for convenience

The program will make several guesses.  For each guess, enter Wordle's response
as 5 upper-case characters, composed of B(lack), Y(ellow), or G(reen).

For instance, the guess resulted in GREEN/YELLOW/BLACK/BLACK/GREEN, you would
enter "GYBBYG".
