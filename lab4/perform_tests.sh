#!/usr/bin/bash
if [ -f wynik-z-serwera.txt ]; then
    rm wynik-z-serwera.txt
fi
echo 'Performing tests...'
./mini-udpcat.py 127.0.0.1 2020 < test-dane.txt >> wynik-z-serwera.txt
diff -q wynik-z-serwera.txt test-wynik.txt
if [ $? -eq 0 ];
then
	echo 'Tests finished sucessfully!'
else
	echo 'Tests failed'
fi
