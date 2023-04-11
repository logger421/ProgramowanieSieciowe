I to byłoby wszystko, jeśli chodzi o serwer datagramowy. Na dzisiejszych zajęciach przechodzimy od protokołów przesyłających dane w datagramach do protokołów korzystających z transportu strumieniowego. Dalej będziemy rozważać problem dodawania i odejmowania liczb naturalnych, w którym wyniki są liczbami całkowitymi, ale teraz zapytania i odpowiedzi będą przesyłane za pomocą TCP.

Zadania (głównie praca koncepcyjna):
```
1) Napisz specyfikację strumieniowego protokołu wartościowania wyrażeń. Dopuść możliwość przesyłania przez jedno połączenie wielu wyrażeń do policzenia i wielu odpowiedzi (obliczonych wyników albo komunikatów o wystąpieniu błędu). Zastanów się, czego użyć jako terminatora mówiącego „w tym miejscu kończy się zapytanie” — dwuznaku \r\n, tak jak w wielu innych protokołach sieciowych? A może czegoś innego (ale wtedy miej jakieś uzasadnienie odejścia od powszechnie przyjętej konwencji)? Czy odpowiedzi serwera będą używać takiego samego terminatora?

Rozważ, czy trzeba do specyfikacji dodawać warunek ograniczający długość przesyłanych przez klienta zapytań, np. 1024 bajty łącznie z terminatorem. To ułatwiłoby implementowanie serwera, bo dzięki temu programista piszący serwer mógłby zadeklarować roboczy bufor o rozmiarze 1024 bajtów i to na pewno wystarczyłoby, aby wczytać do niego całe zapytanie. Ale czy to jest niezbędne? Czy problem wartościowania wyrażeń arytmetycznych wymaga, aby serwer odebrał całe zapytanie, zanim zacznie je przetwarzać?
```
```
2) Zastanów się nad algorytmem serwera. Będzie on musiał być bardziej złożony niż w przypadku serwera UDP. Tam pojedyncza operacja odczytu zawsze zwracała jeden kompletny datagram, czyli jedno kompletne wyrażenie. W przypadku połączeń TCP niestety tak łatwo nie jest.

Po pierwsze, jeśli klient od razu po nawiązaniu połączenia wysłał kilka zapytań jedno za drugim, to serwer może je odebrać sklejone ze sobą. Pojedyncza operacja odczytu ze strumienia może np. zwrócić 15 bajtów odpowiadających znakom 2+2\r\n10+22+34\r\n — jak widać, są to dwa wyrażenia. Serwer w odpowiedzi powinien zwrócić 4\r\n66\r\n.

Po drugie, operacja odczytu może zwrócić tylko początkową część zapytania. Kod serwera musi wtedy ponownie wywołać read(). Takie ponawianie odczytów i odbieranie kolejnych fragmentów wyrażenia musi trwać aż do chwili odebrania \r\n — dopiero wtedy wiemy, że dotarliśmy do końca zapytania.

Po trzecie, mogą się zdarzyć oba powyższe przypadki równocześnie. Serwer może np. odczytać ze strumienia 9 bajtów odpowiadających znakom 2+2\r\n10+2.
```
```
3) Spróbuj rozpisać w formie pseudokodu algorytm serwera obsługujący powyższe komplikacje i starannie przeanalizuj, czy na pewno poradzi on sobie nawet przy założeniu maksymalnie złej woli ze strony klienta.

Polecam Państwa uwadze przedstawioną na wykładzie koncepcję automatu przetwarzającego kolejne bajty z wejścia. Automat nie potrzebuje bufora z kompletnym zapytaniem, po prostu konsumuje bajty w miarę tego jak nadchodzą, więc będzie w stanie przetwarzać dowolnie długie zapytania. Użycie automatu powinno wielce ułatwić zaprojektowanie poprawnie działającego algorytmu.
```
```
4) (nieobowiązkowe) Jeśli chcesz, możesz zacząć implementować w C/C++ taki algorytm. Zdobyte doświadczenie i napisany kod przydadzą się na następnych zajęciach.
```