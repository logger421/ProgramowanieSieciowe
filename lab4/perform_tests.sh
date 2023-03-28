#!/usr/bin/bash
if [ -f wynik-z-serwera.txt ]; then
    rm wynik-z-serwera.txt
fi
echo 'Performing tests...'
files=(
    test-add.txt
    test-subtract.txt
    test-divide.txt
    test-multiply.txt
    test-overflow.txt
    test-underflow.txt
)

results=(
  add-wynik.txt
  subtract-wynik.txt
  divide-wynik.txt
  multiply-wynik.txt
  overflow-wynik.txt
  underflow-wynik.txt
)

for file in "${files[@]}"; do
  ./mini-udpcat.py 127.0.0.1 2020 < tests/"$file" > results/wynik-z-serwera.txt

  postfix=$(echo "$file" | grep -oP "(?<=-).*?(?=\.)")

  diff -q results/wynik-z-serwera.txt tests/"$postfix"-wynik.txt

  if [ $? -eq 0 ];
  then
  	echo "- TEST "$file" finished sucessfully!"
  else

  	echo "- TEST "$file" failed"
  fi
done


