# /bin/bash

# Name  : Junhong Wang                                                         
# ID    : 504941113                                                            
# Email : oneone@g.ucla.edu

errors=0
echo "check" > INPUT 

# check stdin
echo "... checking sdtin"
./lab0 < INPUT > OUTPUT
cmp INPUT OUTPUT > /dev/null
if [ $? -eq 0 ]
then
	echo "	data comparison ... OK"
else
	echo "	data comparison ... FAILURE"
	errors+=1
fi

# check input option
echo "... checking input option"
./lab0 --input=INPUT > OUTPUT
cmp INPUT OUTPUT > /dev/null
if [ $? -eq 0 ]
then
	echo "	data comparison ... OK"
else
	echo "	data comparison ... FAILURE"
	errors+=1
fi

# check output option
echo "... checking ouput option"
./lab0 --output=OUTPUT < INPUT
cmp INPUT OUTPUT > /dev/null
if [ $? -eq 0 ]
then
	echo "	data comparison ... OK"
else
	echo "	data comparison ... FAILURE"
	errors+=1
fi

# check input/output option
echo "... checking input/output option"
./lab0 --input=INPUT --output=OUTPUT
cmp INPUT OUTPUT > /dev/null
if [ $? -eq 0 ]
then
	echo "	data comparison ... OK"
else
	echo "	data comparison ... FAILURE"
	errors+=1
fi

# check segfault option
echo "... cheking segfault option"
./lab0 --segfault < INPUT > OUTPUT
if [ $? -eq 139 ]
then
	echo "	exit status ... OK"
else
	echo "	exit status ... FAILURE"
	errors+=1
fi

# check segfault/catch option
echo "... checking segfault/catch option"
./lab0 --segfault --catch < INPUT > OUTPUT
if [ $? -eq 4 ]
then
	echo "	exit status ... OK"
else
	echo "	exit status ... FAILURE"
	errors+=1
fi

# check copy successful
echo "checking copy successful"
./lab0 < INPUT > OUTPUT
if [ $? -eq 0 ]
then
	echo "	exit status ... OK"
else
	echo "	exit status ... FAILURE"
	errors+=1
fi

# check unrecognized argument
echo "checking unrecognized argument"
./lab0 --unrecognized < INPUT > OUTPUT
if [ $? -eq 1 ]
then
	echo "	exit status ... OK"
else
	echo "	exit status ... FAILURE"
	errors+=1
fi

# check invalid input file
echo "checking invalid input file"
./lab0 --input=INVALID > OUTPUT
if [ $? -eq 2 ]
then
	echo "	exit status ... OK"
else
	echo "	exit status ... FAILURE"
	errors+=1
fi

# check invalid output file
echo "checking invalid output file"
chmod u-w OUTPUT
./lab0 --output=OUTPUT < INPUT
if [ $? -eq 3 ]
then
	echo "	exit status ... OK"
else
	echo "	exit status ... FAILURE"
	errors+=1
fi

echo "total number of errors: $errors"

rm -f INPUT OUTPUT
