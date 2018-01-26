clear

echo 'running '"$1""..."

cd userprog
./nachos -d e -x ../test/"$1"

cd ..