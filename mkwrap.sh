echo "#!/bin/sh" > $3
echo "$1 -r $2 /bin/$4 \$@" >> $3
echo "exit 0" >> $3
chmod 755 $3
