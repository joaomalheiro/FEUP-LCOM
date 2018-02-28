#Runs the robinix service, with passed in arguments
#Use like ./run.sh guard_move, for example

#This is used to concatenate all arguments into a space separated string (if we wish to pass more than 1 argument to the service)

str="$*"

service run `pwd`/robinix -args "$str"
