#!/bin/bash

# input
rm -f a.out down_temp* output.txt
src=$1;
portnum=$2;
echo $src
echo $portnum

H1="HW: 2"
H2="PROF: LEE"


if [[ "$src" =~ ".cpp" ]];
then
	g++ $src
	./a.out $portnum > output.txt &
elif [[ $src =~ ".c" ]];
then
	gcc $src 
	./a.out $portnum > output.txt &
elif [[ $src =~ ".java" ]];
then
	javac $src
	sleep 1
	cname=$(ls HW2*.class | sed 's/.class//')
	java $cname $portnum > output.txt &
elif [[ $src =~ ".py" ]];
then
	python3 $src $portnum > output.txt &
else
	echo "not supported" $src
fi

sleep 1

wget -O down_temp_biga --header="$H1" --header="$H2" -t 2 -T 10 http://localhost:$2/biga.html
sleep 1
wget -O down_temp_a --header="$H1" --header="$H2" -t 2 -T 10 http://localhost:$2/a.html
sleep 1
wget -O down_temp_b --header="$H1" --header="$H2" -t 2 -T 10 http://localhost:$2/b.html
sleep 1
wget -O down_temp_pal --header="$H1" --header="$H2" -t 2 -T 10 http://localhost:$2/palladio.jpg

#killall -q a.out python3 java
killall -q a.out python3 java

succpat=0
succcnt=0
diff biga.html down_temp_biga
if [[ $? == 0 ]];
then
	let 'succpat=succpat+1'
	let 'succcnt=succcnt+1'
fi
diff a.html down_temp_a
if [[ $? == 0 ]];
then
	let 'succpat=succpat+2'
	let 'succcnt=succcnt+1'
fi
diff b.html down_temp_b
if [[ $? == 0 ]];
then
	let 'succpat=succpat+4'
	let 'succcnt=succcnt+1'
fi

cmp palladio.jpg down_temp_pal
if [[ $? == 0 ]];
then
	let 'succpat=succpat+8'
	let 'succcnt=succcnt+1'
fi
grep "User-Agent: Wget" output.txt
if [[ $? == 0 ]];
then
	let 'succpat=succpat+16'
	let 'succcnt=succcnt+1'
fi
grep "7 headers" output.txt
if [[ $? == 0 ]];
then
	let 'succpat=succpat+32'
	let 'succcnt=succcnt+1'
fi

# show the result
echo "Result: " $succpat $succcnt $src

