#Copying configuration file
cp conf/robinix /etc/system.conf.d/
#Creating directories for storing game assets and files
mkdir /home/Robinix
mkdir /home/Robinix/scores
#Copying assets into the easier to access directory
cp -vr res/ /home/Robinix/
#Giving permissions to the other scripts (compile and run)
chmod +x compile.sh
chmod +x run.sh
