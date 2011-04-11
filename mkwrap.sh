v6run=$1
v6root=$2
cmd=$3$4
v6cmd=$2/bin/$4

echo "#!/bin/sh" > $cmd
echo "$v6run -r $v6root $v6cmd \$@" >> $cmd
echo "exit 0" >> $cmd

chmod 755 $cmd
