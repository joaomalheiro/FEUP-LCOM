#Entering the source directory
cd src
#Compiling the code
make clean; make
#Extracting the executable file for it to be run by run.sh
mv -f robinix ../
#Removing unnecessary debug symbols #not yet being used
cd ..
strip --strip-all robinix
