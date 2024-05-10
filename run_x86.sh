make
echo "Make done"
./run.sh $1
echo "Run done"
gcc -o a.out x86_code.s -no-pie
echo "Compile done"
./a.out