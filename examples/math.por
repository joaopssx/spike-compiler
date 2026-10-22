// Algumas funções prontas de matemática.
algoritmo "matemática"
var
  lado, angulo, nota : real
inicio
  lado <- 3
  escreval("Hipotenusa de 3 e 4: ", raiz(pot(lado, 2) + pot(4, 2)))
  nota <- 7.45
  escreval("Nota arredondada: ", arredonda(nota))
  escreval("Parte inteira de -7.45: ", trunca(-nota))
  escreval("Piso e teto de 7.45: ", piso(nota), " e ", teto(nota))
  escreval("Distância entre 3 e 10: ", abs(3 - 10))
  angulo <- 3.14159265358979 / 6
  escreval("Seno de 30 graus (vezes 1000): ", arredonda(seno(angulo) * 1000))
  escreval("Logaritmo de 1000: ", log(1000))
  escreval("Um dado: ", aleatorio(1, 6))
fimalgoritmo
