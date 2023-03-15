Zadania:
```
1) Dokończ pisanie par klient-serwer dla TCP/IPv4 oraz UDP/IPv4 (co razem daje cztery programy). Przetestuj czy działają poprawnie gdy klient i serwer są uruchomione na dwóch różnych komputerach w SPK. Wymaga to znajomości adresu IP przydzielonego komputerowi, na którym uruchamiany jest serwer — można go znaleźć w wynikach polecenia ip address show.
```
```
2) Sprawdź co się dzieje, gdy podasz zły adres IP albo zły numer portu serwera. Czy jądro systemu operacyjnego daje nam w jakiś sposób o tym znać? Jeśli tak, to jak długo trzeba czekać, aż jądro poinformuje nasz proces o wystąpieniu błędu?

Pamiętaj, że protokoły sieciowe z korekcją błędów wykonują wielokrotne retransmisje pakietów w zwiększających się odstępach czasu. Może to zająć nawet kilkadziesiąt minut. Nie pomyl sytuacji „proces zawiesza się na pięć minut zanim jądro zwróci -1” z sytuacją „zawiesza się na stałe”.

Jeśli któryś z klientów może się zawiesić czekając w nieskończoność na odpowiedź z nieistniejącego serwera, to popraw jego kod aby tego nie robił. W slajdach z wykładu są pokazane funkcje, które pozwalają na wykonywanie operacji we-wy z timeoutem (możecie go Państwo ustawić np. na 10 sekund).
```
```
3) Przeanalizuj niniejszą specyfikację protokołu dodawania i odejmowania liczb naturalnych. Czy jest ona jednoznaczna, czy też może zostawia pewne rzeczy niedopowiedziane?

Komunikacja pomiędzy klientem a serwerem odbywa się przy pomocy datagramów. Klient wysyła datagram zawierający wyrażenie zbudowane z liczb i operatorów dodawania / odejmowania. Serwer odpowiada datagramem zawierającym pojedynczą liczbę (obliczony wynik) bądź komunikat o błędzie.

Zawartość datagramów interpretujemy jako tekst w ASCII. Ciągi cyfr ASCII interpretujemy jako liczby dziesiętne. Datagram może zawierać albo cyfry, plusy i minusy, albo pięć znaków składających się na słowo „ERROR”. Żadne inne znaki nie są dozwolone (ale patrz następny akapit).

Aby ułatwić ręczne testowanie serwera przy pomocy ncat, serwer może również akceptować datagramy mające na końcu dodatkowy znak \n (czyli bajt o wartości 10) albo dwa znaki \r\n (bajty 13, 10). Serwer może wtedy, ale nie musi, dodać \r\n do zwracanej odpowiedzi.
```
```
3) Napisz serwer UDP/IPv4 nasłuchujący na porcie nr 2020 i implementujący powyższy protokół.

Serwer musi weryfikować odebrane dane i zwracać komunikat o błędzie jeśli są one nieprawidłowe w sensie zgodności ze specyfikacją protokołu.

W kodzie używaj zmiennych roboczych któregoś ze standardowych typów całkowitoliczbowych (int, long int, int32_t, itd.). Co za tym idzie, odebrany ciąg cyfr będzie mógł reprezentować liczbę zbyt dużą, aby dało się ją zapisać w zmiennej wybranego typu. Może też się zdarzyć, że podczas dodawania bądź odejmowania wystąpi przepełnienie (ang. integer overflow / wraparound). Serwer ma obowiązek wykrywać takie sytuacje i zwracać błąd. Uwadze Państwa polecam pliki nagłówkowe limits.h oraz stdint.h, w których znaleźć można m.in. parę stałych INT_MIN i INT_MAX oraz parę INT32_MIN i INT32_MAX.
```