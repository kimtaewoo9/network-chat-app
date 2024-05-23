#!/bin/bash

# input
rm -f a.out *.JPG *.html
src=$1;

echo $1
echo $1 >> /dev/stderr

succcnt=0
if [[ "$src" =~ ".cpp" ]];
then
	g++ $src
	./a.out < lab.in > test.out
elif [[ $src =~ ".c" ]];
then
	gcc $src 
	./a.out < lab.in > test.out
elif [[ $src =~ ".java" ]];
then
	javac $src
	sleep 1
	cname=$(ls *.class | sed 's/.class//')
	java $cname < lab.in > test.out
elif [[ $src =~ ".py" ]];
then
	python3 $src < lab.in > test.out
else
	echo "not supported" $src
	exit
fi

if [[ $? == 0 ]];
then
	let "succcnt=succcnt+1"
fi

grep "404" test.out
if [[ $? == 0 ]];
then
	let "succcnt=succcnt+2"
fi

echo ""
echo "Binary File Compare"
cmp palladio.JPG palladio.JPG.org

if [[ $? == 0 ]];
then
	let "succcnt=succcnt+4"
fi

echo ""
echo "Text File Compare"

diff index.html index.html.org

if [[ $? == 0 ]];
then
	let "succcnt=succcnt+8"
fi


grep "Only support http, not https" test.out
if [[ $? == 0 ]];
then
	let "succcnt=succcnt+16"
fi

echo "Score: " $succcnt $src

if [[ $succcnt == 31 ]];
then
	echo "Success: " $src
#	mv $src succ
elif [[ $succcnt == 8 ]];
then
	echo "Partial 3: " $src
	#mv $src htmlonly
elif [[ $succcnt == 4 ]];
then
	echo "Partial 2: " $src
	#mv $src htmlonly
elif [[ $succcnt == 2 ]];
then
	echo "Partial 1: " $src
	#mv $src jpgonly
else
	fexist=0
	if [[ -f index.html ]];
	then
		let "fexist=fexist+1"
	fi
	if [[ -f palladio.JPG ]];
	then
		let "fexist=fexist+1"
	fi

	if [[ $fexist == 2 ]];
	then
		echo "FExist: " $src
		#mv $src fexist
	else
		echo "Fail: " $src
		#mv $src fail
	fi
fi

rm -f a.out *.JPG *.html test.out


