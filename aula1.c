#include <stdio.h>

int main() {
    char nome;
    int idade;
    float altura;

    printf("Insira o seu nome:\n");
    scanf("%c", &nome);

    printf("Insira a sua idade:\n");
    scanf("%d", &idade);

    printf("Insira a sua altura:\n");
    scanf("%f", &altura);

    printf("O seu nome é: %c\n", nome);
    printf("O seu idade é: %d\n", idade);
    printf("O seu altura é: %.2f\n", altura);

    return 0;
}