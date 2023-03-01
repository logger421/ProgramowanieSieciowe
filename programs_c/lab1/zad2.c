#include <stdio.h>
const int MAX_SIZE = 50;

void drukuj(int tablica[], int liczba_elementow);
void drukuj_alt(int * tablica, int liczba_elementow);

int main() {
    int liczby[MAX_SIZE];
    int i;
    int input;
    for(i = 0; i < MAX_SIZE; i++) {
        scanf("%d", &input);
        if(input == 0) break;
        liczby[i] = input;
    }
    drukuj(liczby, MAX_SIZE);
    return 0;
}


void drukuj(int tablica[], int liczba_elementow) {
    printf("From drukuj\n");
    int i;
    printf("%4s%12s\n", "i", "tablica[i]");
    for(i = 0; i < liczba_elementow; i++) {
        if(tablica[i] > 10 && tablica[i] < 100) {
            printf("%4d%12d\n", i, tablica[i]);
        }
    }
}

void drukuj_alt(int * tablica, int liczba_elementow) {
    
}