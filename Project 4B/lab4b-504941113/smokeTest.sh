#!/bin/bash

# Name  : Junhong Wang
# ID    : 504941113
# Email : oneone@g.ucla.edu

let errors=0

# start
echo "Smoke Test Starts ..."

# check --log
echo "... checking --log"
./lab4b --log=LOGFILE > STDOUT <<-EOF
OFF
EOF

if [ -s LOGFILE ]
then
	echo "... SUCCESS"
else
	errors+=1
	echo "... FAIL"
fi

# check SCALE=F
echo "... checking SCALE=F"
./lab4b --log=LOGFILE > STDOUT <<-EOF
SCALE=F
OFF
EOF

grep "SCALE=F" LOGFILE > /dev/null
if [ $? -eq 0 ]
then
	echo "... SUCCESS"
else
	errors+=1
	echo "... FAIL"
fi

# check SCALE=C
echo "... checking SCALE=C"
./lab4b --log=LOGFILE > STDOUT <<-EOF
SCALE=C
OFF
EOF

grep "SCALE=C" LOGFILE > /dev/null
if [ $? -eq 0 ]
then
	echo "... SUCCESS"
else
	errors+=1
	echo "... FAIL"
fi

# check PERIOD=3
echo "... checking PERIOD=3"
./lab4b --log=LOGFILE > STDOUT <<-EOF
PERIOD=3
OFF
EOF

grep "PERIOD=3" LOGFILE > /dev/null
if [ $? -eq 0 ]
then
	echo "... SUCCESS"
else
	errors+=1
	echo "... FAIL"
fi

# check STOP / START
echo "... checking STOP / START"
./lab4b --log=LOGFILE > STDOUT <<-EOF
STOP
START
OFF
EOF

grep "STOP" LOGFILE > /dev/null
if [ $? -eq 0 ]
then
	echo "... SUCCESS"
else
	errors+=1
	echo "... FAIL"
fi

grep "START" LOGFILE > /dev/null
if [ $? -eq 0 ]
then
	echo "... SUCCESS"
else
	errors+=1
	echo "... FAIL"
fi

# check LOG
echo "... checking LOG"
./lab4b --log=LOGFILE > STDOUT <<-EOF
LOG Hello World
OFF
EOF

grep "Hello World" LOGFILE > /dev/null
if [ $? -eq 0 ]
then
	echo "... SUCCESS"
else
	errors+=1
	echo "... FAIL"
fi

# report the result of smoke test
if [ $errors -eq 0 ]
then
	echo "... PASSED ALL CHECKS"
else
	echo "... FAILED TO PASS TESTS WITH $errors ERRORS"
fi

# cleanup
rm STDOUT LOGFILE

# end
echo "Smoke Test Ends ..."







