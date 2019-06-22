# /bin/bash

# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

./lab3a trivial.img | sort > TARGET

cat trivial.csv | sort > EXPECTED

diff TARGET EXPECTED

if [ $? -eq 0 ]
then
	echo "passed the test"
else
	echo "failed to pass the test"
fi

rm -f TARGET EXPECTED
