#!/bin/bash

NC_FLAGS=$1
USERPASS_FILE="/home/bob/vanpowerscripts/prod/loginmysql.conf"
IMPORT_DATA_FILE="/home/bob/vanpowerscripts/prod/lastimport.my.sql"

echo started $(date)
if [ "$NC_FLAGS" == "" ]; then
    $NC_FLAGS="-lvv"
    echo nc flags = $NC_FLAGS
else
    echo making request to receive data to/from $NC_FLAGS
fi

echo creating backup of existing data to beforeimportbackup.bak.my.sql
# mysqldump -u root -pletmein --databases vanpower > beforeimportbackup.bak.my.sql
mysqldump -u dbcreator -p'[user to create temporary database for import/export]' --databases vanpower > beforeimportbackup.bak.my.sql

echo creating ALT van power database
mysql -u dbcreator -p'[user to create temporary database for import/export]' < createvanpoweralt.my.sql

echo listening to new records to import into vanpoweralt, on port 10000
echo "ack $(date)" | nc $NC_FLAGS 10000 > $IMPORT_DATA_FILE
echo processing data
cat $IMPORT_DATA_FILE | mysql --defaults-extra-file=$USERPASS_FILE vanpoweralt

echo Data received... Processing data...
mysql --defaults-extra-file=$USERPASS_FILE vanpower < copylaterdata.my.sql

echo finishd $(date)
