W tym tygodniu zajmiemy się testowaniem. Przy implementowaniu protokołów sieciowych samo napisanie kodu to tylko początek pracy — potem trzeba sprawdzić, czy ten kod jest zgodny ze specyfikacją protokołu. Trzeba przetestować, czy akceptuje on wszystkie zapytania, które wg specyfikacji są poprawne, czy wykrywa i odrzuca wszystkie te, które są niepoprawne, i czy zwracane odpowiedzi mają zgodny ze specyfikacją format.

Przy okazji kilka słów o wykrywaniu przepełnień arytmetycznych. Ktoś kiedyś próbował oddać program, w którym był taki kod:
```c
int wynik, i;

// tu coś było robione z tymi dwoma zmiennymi

wynik = wynik + i;
if (wynik > INT_MAX) {   // nie działa
// obsłuż przepełnienie
}

// Albo może to było napisane tak:

int wynik, i;

// tu coś było robione z tymi dwoma zmiennymi

if (wynik + i > INT_MAX) {   // też nie działa
// obsłuż przepełnienie
} else {
wynik = wynik + i;
}
```
Jeśli nie jest dla Państwa oczywiste, dlaczego powyższe próby wykrywania przepełnienia nie mają sensu, to się proszę zastanówcie: w której dokładnie chwili podczas wykonywania programu pojawia się przepełnienie? Która z operacji wykonywanych przez procesor komputera zwraca błędny wynik i jakie ma to konsekwencje dla następnych operacji?

Druga sprawa: niektóre osoby do konwersji ciągu cyfr ASCII na liczbę wykorzystują funkcje z biblioteki standardowej, takie jak atoi, strtol czy strtoull. Czy sprawdziliście, jak te funkcje się zachowują gdy ta liczba jest zbyt duża (np. gdy dostaną ciąg złożony z czterdziestu cyfr „7”)? Czy kod serwera sprawdza status zwracany przez wywoływaną funkcję i w razie błędu odpowiednio reaguje?

Zadania:
```
1) Przetestuj ręcznie serwer UDP wartościujący wyrażenia. Jeśli akceptuje końcowe \r\n, to możesz to zrobić uruchamiając

ncat --udp --crlf 127.0.0.1 2020
albo
socat stdio udp4:127.0.0.1:2020,crlf
i wpisując kolejne zapytania z klawiatury.

Zapytanie bez końcowego \r\n czy też \n można wygenerować np. poleceniem printf podłączonym do wejścia socata:

printf "1+23-8" | socat -t 5.0 stdio udp4:127.0.0.1:2020
Przełącznik -t 5.0 nakazuje socatowi odczekać pięć sekund po zakończeniu wysyłania danych do serwera i zakończyć działanie. Jakiś timeout jest niezbędny, bo socat nie wie, że serwer zwróci dokładnie jeden datagram. W ogólnym przypadku odpowiedź z serwera UDP może się przecież składać z wielu datagramów, a na poziomie bezpołączeniowego protokołu transportowego, jakim jest UDP, nie ma po czym poznać że już je wszystkie odebrano.
```
```
2) Często spotykanym w poprzednich latach błędem było zwracanie przez serwer dodatkowych bajtów o wartości zero, bo np. ktoś w kodzie zadeklarował sobie char wynik[20], użył sprintf aby „wydrukować” do tej tablicy tekstową reprezentację wyniku (która zajęła tylko kilka początkowych elementów tablicy), a potem przez pomyłkę wysłał klientowi całą 20-bajtową tablicę. Ten błąd łatwo przegapić, bo bajty o wartości zero są niewidoczne gdy się je wyświetla na ekranie.

Aby sprawdzić jakie dokładnie bajty są w strumieniu danych trzeba ten strumień wysłać nie wprost na ekran, lecz np. na wejście programu od. Proszę porównać to, co wypisują dwa poniższe polecenia:

printf "abc ijk\0xyz\n"

printf "abc ijk\0xyz\n" | od -A d -t u1 -t c
Użyte przełączniki nakazują wyświetlić kolejne bajty w postaci dziesiętnej oraz jako znaki ASCII (bajty odpowiadające niedrukowalnym znakom kontrolnym są wyświetlane jako sekwencje z backslashem na początku).

Proszę spróbować zapisać zwrócone przez serwer dane do pliku, a potem ten plik wyświetlić za pomocą od:

ncat --udp --crlf 127.0.0.1 2020 > wynik-z-serwera.txt

printf "2+2" | socat -t 5.0 stdio udp4:127.0.0.1:2020 > wynik-z-serwera.txt

od -A d -t u1 -t c < wynik-z-serwera.txt
```
```
3) Możliwość przekierowania równocześnie wejścia i wyjścia socata można wykorzystać do stworzenia powtarzalnych testów. Załóżmy, że w pliku test-dane.txt jest ciąg bajtów składający się na zapytanie testowe. Uruchamiamy

socat -t 5.0 stdio udp4:127.0.0.1:2020 < test-dane.txt > wynik-z-serwera.txt
Jeśli przygotowaliśmy również plik test-wynik.txt, to przy pomocy poleceń cmp albo diff można łatwo porównać zawartość pliku wynik-z-serwera.txt ze wzorcowym wynikiem.

socat nie radzi sobie z datagramami mającymi długość zero bajtów, nie da się więc przy jego pomocy wysłać pustego zapytania. Nie ma też jak odróżnić sytuacji, gdy serwer zwrócił pustą odpowiedź, od sytuacji gdy w ogóle żadna odpowiedź nie została zwrócona. Z tych powodów zamiast socata możesz chcieć użyć programu mini-udpcat.py, który został napisany specjalnie na potrzeby tych zajęć.
```
```
4) Przygotuj kilka par plików z przykładowymi zapytaniami i oczekiwanymi wynikami. Uwzględnij także błędne zapytania, na które odpowiedzią powinno być „ERROR”.
Wymień się tymi plikami z dwiema-trzema innymi osobami z grupy. Sprawdź, czy Twój serwer poprawnie obsługuje zapytania przygotowane przez inne osoby. Jeśli nie, to spróbujcie wspólnie ustalić przyczynę: różnice w rozumieniu specyfikacji protokołu, korzystanie w teście z opcjonalnej funkcjonalności, którą nie wszystkie serwery muszą implementować (u nas: \r\n na końcu datagramu), błędy w kodzie serwera, coś innego?
```
```
5) (nieobowiązkowe, ale przydatne) Przygotuj sobie narzędzie automatycznie testujące sumator w oparciu o powyższe pliki z zapytaniami i odpowiedziami. Na przykład prosty skrypt w języku uniksowej powłoki, wywołujący polecenia używane w poprzednich punktach. Możesz też napisać program w C albo innym języku, wczytujący te pliki oraz komunikujący się przez gniazdko z serwerem. Zanim jednak się zabierzesz za jego pisanie, to lepiej sprawdź czy w sieci nie da się znaleźć gotowego narzędzia do testowania usług UDP — wielce możliwe, że ktoś już coś takiego zaimplementował.
```